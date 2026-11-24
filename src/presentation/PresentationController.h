#pragma once

#include "core/Types.h"
#include "core/ShowDocument.h"
#include "persistence/ShowRepository.h"
#include "crdt/Document.h"
#include "crdt/SyncSession.h"
#include "remote/RemoteServer.h"
#include "remote/RemoteDiscovery.h"
#include "media/MediaLibrary.h"
#include "media/Thumbnailer.h"
#include "output/OutputManager.h"
#include "timeline/TimelineEngine.h"
#include "scripture/ScriptureProvider.h"
#include "timers/TimerManager.h"
#include "templates/TemplateLibrary.h"
#include "overlays/OverlayManager.h"
#include "audio/AudioPlayer.h"
#include "triggers/TriggerManager.h"
#include "backup/BackupManager.h"
#include "providers/PlanningCenterProvider.h"
#include "cloud/CloudSyncClient.h"
#include "chords/ChordEngine.h"

#include <QObject>
#include <QVariantList>
#include <QJsonObject>
#include <QTimer>

class QQmlEngine;

namespace ps::presentation {

// The application's central controller. QML talks to this object through
// properties and Q_INVOKABLE methods; the phone remote talks to it via the
// RemoteServer's apiRequested signal.
class PresentationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString activeShowName READ activeShowName NOTIFY showChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY stateChanged)
    Q_PROPERTY(int slideCount READ slideCount NOTIFY slidesChanged)
    Q_PROPERTY(QVariantList slides READ slides NOTIFY slidesChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged)
    Q_PROPERTY(QVariantList mediaFiles READ mediaFiles NOTIFY mediaFilesChanged)
    Q_PROPERTY(QString remoteAddress READ remoteAddress NOTIFY remoteChanged)
    Q_PROPERTY(bool remoteEnabled READ remoteEnabled NOTIFY remoteChanged)
    Q_PROPERTY(bool modified READ modified NOTIFY showChanged)
    Q_PROPERTY(QString clockText READ clockText NOTIFY tick)
    Q_PROPERTY(bool timelinePlaying READ timelinePlaying NOTIFY timelineStateChanged)
    Q_PROPERTY(qint64 timelinePosition READ timelinePosition NOTIFY timelinePositionChanged)
    Q_PROPERTY(QVariantList timelineActions READ timelineActions NOTIFY timelineActionsChanged)
    Q_PROPERTY(QString scriptureReference READ scriptureReference NOTIFY scriptureChanged)
    Q_PROPERTY(QString scriptureText READ scriptureText NOTIFY scriptureChanged)
    Q_PROPERTY(QStringList bibleNames READ bibleNames NOTIFY biblesChanged)
    Q_PROPERTY(QVariantList timers READ timers NOTIFY timersChanged)
    Q_PROPERTY(QVariantList templates READ templates NOTIFY templatesChanged)
    Q_PROPERTY(QVariantList overlays READ overlays NOTIFY overlaysChanged)
    Q_PROPERTY(QVariantList scenes READ scenes NOTIFY overlaysChanged)
    Q_PROPERTY(QVariantList activeOverlays READ activeOverlays NOTIFY overlaysChanged)
    Q_PROPERTY(QVariantList audioTracks READ audioTracks NOTIFY audioChanged)
    Q_PROPERTY(QString audioTrack READ audioTrack NOTIFY audioChanged)
    Q_PROPERTY(bool audioPlaying READ audioPlaying NOTIFY audioChanged)
    Q_PROPERTY(QVariantList triggers READ triggers NOTIFY triggersChanged)
    Q_PROPERTY(bool midiConnected READ midiConnected NOTIFY triggersChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(bool cloudConnected READ cloudConnected NOTIFY cloudChanged)
    Q_PROPERTY(QVariantList backups READ backups NOTIFY backupsChanged)
    Q_PROPERTY(QVariantList plans READ plans NOTIFY plansChanged)

public:
    explicit PresentationController(QObject* parent = nullptr);

    QString activeShowName() const;
    int currentIndex() const;
    int slideCount() const;
    QVariantList slides() const { return m_slides; }
    QVariantList searchResults() const { return m_searchResults; }
    QVariantList mediaFiles() const { return m_mediaFiles; }
    QString remoteAddress() const { return m_remoteAddress; }
    bool remoteEnabled() const;
    bool modified() const;
    QString clockText() const;
    bool timelinePlaying() const;
    qint64 timelinePosition() const;
    QVariantList timelineActions() const;
    QString scriptureReference() const;
    QString scriptureText() const;
    QStringList bibleNames() const;
    QVariantList timers() const { return m_timers.timers(); }
    QVariantList templates() const { return m_templates.templates(); }
    QVariantList overlays() const { return m_overlays.overlays(); }
    QVariantList scenes() const { return m_overlays.scenes(); }
    QVariantList activeOverlays() const { return m_overlays.activeOverlays(); }
    QVariantList audioTracks() const { return m_audio.tracks(); }
    QString audioTrack() const { return m_audio.currentName(); }
    bool audioPlaying() const { return m_audio.playing(); }
    QVariantList triggers() const { return m_triggers.triggers(); }
    bool midiConnected() const { return m_triggers.midiConnected(); }
    bool recording() const { return m_output.recording(); }
    bool cloudConnected() const { return m_cloud.connected(); }
    QVariantList backups() const { return m_backup.backups(); }
    QVariantList plans() const { return m_planningCenter.plans(); }

    ShowDocument& document() { return m_document; }
    output::OutputManager& output() { return m_output; }
    ShowRepository& repository() { return m_repository; }
    crdt::Document& crdtDocument() { return m_crdt; }

    Q_INVOKABLE void newShow(const QString& name = {});
    Q_INVOKABLE bool openShow(const QString& id);
    Q_INVOKABLE bool saveShow();
    Q_INVOKABLE bool importShowFile(const QString& path);

    Q_INVOKABLE QString addLyricSlide(const QString& group, const QString& text,
                                      const QString& key = {});
    Q_INVOKABLE QString addMediaSlide(const QString& group, const QString& path);
    Q_INVOKABLE bool deleteSlide(const QString& id);
    Q_INVOKABLE bool duplicateSlide(const QString& id);
    Q_INVOKABLE void moveSlide(int from, int to);

    Q_INVOKABLE void nextSlide();
    Q_INVOKABLE void previousSlide();
    Q_INVOKABLE void goToSlide(int index);
    Q_INVOKABLE void clearOutput();
    Q_INVOKABLE void restoreOutput();
    Q_INVOKABLE void startPresenting();
    Q_INVOKABLE void stopPresenting();

    Q_INVOKABLE void search(const QString& query);
    Q_INVOKABLE bool openSearchResult(int row);

    Q_INVOKABLE bool startRemote(const QString& password = {}, quint16 port = 5510);
    Q_INVOKABLE void stopRemote();
    Q_INVOKABLE bool openDisplay(int screenIndex);
    Q_INVOKABLE void closeDisplays();
    Q_INVOKABLE void scanMedia(const QString& directory);

    // Timeline
    Q_INVOKABLE void startTimeline();
    Q_INVOKABLE void pauseTimeline();
    Q_INVOKABLE void stopTimeline();
    Q_INVOKABLE void seekTimeline(qint64 milliseconds);
    Q_INVOKABLE void addTimelineAction(const QString& name, const QString& type,
                                       qint64 timeMs, double durationSeconds);
    Q_INVOKABLE bool removeTimelineAction(const QString& id);

    // Scripture
    Q_INVOKABLE QVariantList scriptureSearch(const QString& query);
    Q_INVOKABLE bool openScripture(const QString& reference);
    Q_INVOKABLE bool scriptureNext();
    Q_INVOKABLE bool scripturePrevious();
    Q_INVOKABLE bool setActiveBible(const QString& id);

    // Timers & templates
    Q_INVOKABLE QString startTimer(const QString& name, double seconds, int warnSeconds = 0);
    Q_INVOKABLE bool toggleTimer(const QString& id);
    Q_INVOKABLE bool stopTimer(const QString& id);
    Q_INVOKABLE void stopAllTimers();
    Q_INVOKABLE bool applyTemplate(const QString& slideId, const QString& templateId);
    Q_INVOKABLE QString createTemplate(const QString& name, const QString& slideId);

    // Overlays / scenes
    Q_INVOKABLE void toggleOverlay(const QString& id);
    Q_INVOKABLE void clearOverlays();
    Q_INVOKABLE bool startScene(const QString& id);

    // Audio & triggers
    Q_INVOKABLE QString addAudioTrack(const QString& path);
    Q_INVOKABLE bool removeAudioTrack(const QString& id);
    Q_INVOKABLE bool transposeShow(int semitones);
    Q_INVOKABLE QString detectKey() const;
    Q_INVOKABLE void playAudio();
    Q_INVOKABLE void pauseAudio();
    Q_INVOKABLE void stopAudio();
    Q_INVOKABLE void nextAudio();
    Q_INVOKABLE void previousAudio();
    Q_INVOKABLE void setAudioVolume(double volume);
    Q_INVOKABLE void setAudioMuted(bool muted);
    Q_INVOKABLE void seekAudio(qint64 ms);
    Q_INVOKABLE bool connectMidi(int device = 0);
    Q_INVOKABLE void disconnectMidi();
    Q_INVOKABLE QString addMidiTrigger(const QString& action, int note, bool control = false);
    Q_INVOKABLE QString addKeyTrigger(const QString& action, const QString& key);
    Q_INVOKABLE void fireTriggerKey(const QString& key);

    // Recording
    Q_INVOKABLE bool startRecording(const QString& path);
    Q_INVOKABLE void stopRecording();

    // Backup / content providers / cloud
    Q_INVOKABLE QString backupNow(const QString& label = {});
    Q_INVOKABLE bool restoreBackup(const QString& id);
    Q_INVOKABLE void configurePlanningCenter(const QString& appId, const QString& secret);
    Q_INVOKABLE void fetchPlans(const QString& from = {}, const QString& to = {});
    Q_INVOKABLE bool importPlan(const QString& planId);
    Q_INVOKABLE void connectCloud(const QString& url, const QString& room, const QString& token = {});
    Q_INVOKABLE void disconnectCloud();

    void attachEngine(QQmlEngine* engine);

    Q_INVOKABLE QVariantMap servicePlan() const;
    Q_INVOKABLE bool openProject(const QString& id);
    Q_INVOKABLE void nextServiceItem();

signals:
    void showChanged();
    void slidesChanged();
    void stateChanged();
    void searchResultsChanged();
    void mediaFilesChanged();
    void remoteChanged();
    void tick();
    void timelineStateChanged();
    void timelinePositionChanged(qint64 ms);
    void timelineActionsChanged();
    void scriptureChanged();
    void biblesChanged();
    void timersChanged();
    void templatesChanged();
    void overlaysChanged();
    void audioChanged();
    void triggersChanged();
    void recordingChanged();
    void cloudChanged();
    void backupsChanged();
    void plansChanged();
    void errorOccurred(const QString& message);
    void toast(const QString& message);

private slots:
    void onDocumentChanged();
    void onApiRequested(const QString& api, const QJsonObject& data,
                        const QString& clientId, bool wantsResponse);
    void updateClock();
    void onTimelineAction(const ps::TimelineAction& action);
    void onTriggerAction(const QString& action, const QJsonObject& data);

private:
    void rebuildSlides();
    void pushOutput();
    void syncShowToCrdt();
    QVariantMap slideToMap(const Id& id, int index) const;
    QJsonArray showsForRemote() const;
    void setLayoutFromRemote(const QString& layoutId);
    void loadTimelineForShow();
    QQmlEngine* m_engine = nullptr;

    Database m_database;
    ShowRepository m_repository;
    ShowDocument m_document;
    crdt::Document m_crdt;
    crdt::SyncSession m_sync;
    remote::RemoteServer m_remote;
    remote::RemoteDiscovery m_discovery;
    media::MediaLibrary m_mediaLibrary;
    media::Thumbnailer m_thumbnailer;
    output::OutputManager m_output;
    timeline::TimelineEngine m_timeline;
    scripture::ScriptureProvider m_scripture;
    timers::TimerManager m_timers;
    templates::TemplateLibrary m_templates;
    overlays::OverlayManager m_overlays;
    audio::AudioPlayer m_audio;
    triggers::TriggerManager m_triggers;
    backup::BackupManager m_backup;
    providers::PlanningCenterProvider m_planningCenter{&m_repository};
    cloud::CloudSyncClient m_cloud{m_crdt};

    QVariantList m_slides;
    QVariantList m_searchResults;
    QVariantList m_mediaFiles;
    QVector<Id> m_slideOrder;
    Id m_activeProjectId;
    int m_serviceIndex = -1;
    QString m_remoteAddress;
    QString m_clockText;
    QTimer m_clock;
};

} // namespace ps::presentation
