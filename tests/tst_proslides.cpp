#include <QtTest>
#include <QSignalSpy>
#include <QDir>
#include <QFile>
#include "core/Types.h"
#include "core/ShowFile.h"
#include "core/ShowDocument.h"
#include "persistence/Database.h"
#include "persistence/ShowRepository.h"
#include "crdt/Document.h"
#include "timeline/TimelineEngine.h"
#include "scripture/Bible.h"
#include "timers/TimerManager.h"
#include "templates/TemplateLibrary.h"
#include "overlays/OverlayManager.h"
#include "triggers/TriggerManager.h"
#include "backup/BackupManager.h"
#include "chords/ChordEngine.h"

using namespace ps;

class ProSlidesTest : public QObject
{
    Q_OBJECT

private slots:
    void showFileRoundTrip();
    void lyricSlideParsing();
    void crdtConverges();
    void ftsVietnameseSearch();
    void timelineTriggers();
    void scriptureReferenceParsing();
    void timerLifecycle();
    void templateRoundTrip();
    void overlaySceneSwitch();
    void triggerMapping();
    void backupSnapshot();
    void chordTranspose();
};

void ProSlidesTest::showFileRoundTrip()
{
    Show show;
    show.id = makeId();
    show.name = QStringLiteral(u"Sunday Service");

    Slide slide;
    slide.id = makeId();
    slide.group = QStringLiteral(u"Worship Set");
    slide.notes = QStringLiteral(u"Pray before song");
    Item item;
    item.type = ItemType::Text;
    Line line;
    line.align = QStringLiteral(u"center");
    TextPart part;
    part.value = QStringLiteral(u"Amazing grace");
    part.style = QStringLiteral(u"color:#fff;");
    line.text.push_back(part);
    item.lines.push_back(line);
    slide.items.push_back(item);
    show.slides.insert(slide.id, slide);

    Layout layout;
    layout.id = makeId();
    layout.name = QStringLiteral(u"Main");
    SlideData ref;
    ref.id = slide.id;
    layout.slides.push_back(ref);
    show.layouts.insert(layout.id, layout);
    show.settings.activeLayout = layout.id;

    const auto bytes = encodeShowFile(show);
    const auto result = decodeShowFile(bytes);
    QVERIFY(result.ok);
    QCOMPARE(result.show.id, show.id);
    QCOMPARE(result.show.name, show.name);
    QCOMPARE(result.show.slides.size(), size_t(1));
    QCOMPARE(result.show.slides.begin()->second.items.at(0).plainText(), QStringLiteral(u"Amazing grace"));

    // FreeShow files are [id, Show] tuples.
    const auto doc = QJsonDocument::fromJson(bytes);
    QVERIFY(doc.isArray());
    QCOMPARE(doc.array().size(), 2);
}

void ProSlidesTest::lyricSlideParsing()
{
    ShowDocument doc;
    const Id id = doc.addLyricSlide(QStringLiteral(u"Worship"), QStringLiteral(u"[C]When all [G]see is the battle\nYou see my victory"), QStringLiteral(u"C"));
    QVERIFY(!id.isEmpty());
    const auto& slide = doc.show().slides.at(id);
    QCOMPARE(slide.items.size(), size_t(2));
    QCOMPARE(slide.items.at(0).lines.at(0).chords.size(), size_t(2));
    QCOMPARE(slide.items.at(0).lines.at(0).chords.at(0).key, QStringLiteral(u"C"));
    QCOMPARE(slide.items.at(0).lines.at(0).plainText(), QStringLiteral(u"When all see is the battle"));
    QVERIFY(doc.canUndo());
    QVERIFY(doc.undo());
    QVERIFY(!doc.show().slides.contains(id));
}

void ProSlidesTest::crdtConverges()
{
    crdt::Document a(QStringLiteral(u"actor-a"));
    crdt::Document b(QStringLiteral(u"actor-b"));

    a.set({ QStringLiteral(u"slides"), QStringLiteral(u"s1"), QStringLiteral(u"notes")}, QStringLiteral(u"Hello from A"));
    b.set({ QStringLiteral(u"slides"), QStringLiteral(u"s1"), QStringLiteral(u"notes")}, QStringLiteral(u"Hello from B"));

    const auto aOps = a.operationsSince({});
    const auto bOps = b.operationsSince({});
    QVERIFY(b.applyBatch(aOps));
    QVERIFY(a.applyBatch(bOps));

    QCOMPARE(a.get({ QStringLiteral(u"slides"), QStringLiteral(u"s1"), QStringLiteral(u"notes")}).toString(),
             b.get({ QStringLiteral(u"slides"), QStringLiteral(u"s1"), QStringLiteral(u"notes")}).toString());

    // Edge insertion also converges.
    a.insert({ QStringLiteral(u"order")}, 0, QStringLiteral(u"first"));
    b.applyBatch(a.operationsSince(b.vectorClock()));
    QCOMPARE(a.sequence({ QStringLiteral(u"order")}).size(), b.sequence({ QStringLiteral(u"order")}).size());
}

void ProSlidesTest::ftsVietnameseSearch()
{
    Database db(QStringLiteral(u":memory:"));
    QString error;
    QVERIFY2(db.open(&error), qPrintable(error));
    ShowRepository repo(db);
    repo.setDataDirectory(QDir::tempPath());

    Show show;
    show.id = makeId();
    show.name = QStringLiteral(u"Thờ phượng");
    Slide slide;
    slide.id = makeId();
    slide.group = QStringLiteral(u"Chúa là tình yêu");
    Item item;
    item.type = ItemType::Text;
    Line line;
    TextPart part;
    part.value = QStringLiteral(u"Yêu Chúa hết lòng");
    line.text.push_back(part);
    item.lines.push_back(line);
    slide.items.push_back(item);
    show.slides.insert(slide.id, slide);

    QVERIFY(repo.saveShow(show, &error));
    const auto hits = repo.search(QStringLiteral(u"yeu chua"));
    QVERIFY(hits.size() >= 1);
    QCOMPARE(hits.first().slideId, slide.id);
}

void ProSlidesTest::timelineTriggers()
{
    ps::timeline::TimelineEngine engine;
    Timeline timeline;
    TimelineAction a;
    a.id = makeId();
    a.name = QStringLiteral(u"Next song");
    a.type = QStringLiteral(u"slide");
    a.time = 10;
    timeline.actions.push_back(a);
    engine.setTimeline(timeline);

    QSignalSpy spy(&engine, SIGNAL(actionTriggered(const TimelineAction&)));
    engine.start();
    QTest::qWait(60);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(engine.duration(), qint64(10));
}

void ProSlidesTest::scriptureReferenceParsing()
{
    scripture::Bible bible;
    bible.name = QStringLiteral(u"Test Bible");
    scripture::Book book;
    book.number = 43;
    book.name = QStringLiteral(u"John");
    book.abbreviation = QStringLiteral(u"Jn");
    scripture::Chapter chapter;
    chapter.number = 3;
    scripture::Verse verse;
    verse.number = 16;
    verse.text = QStringLiteral(u"For God so loved the world");
    chapter.verses.append(verse);
    book.chapters.append(chapter);
    bible.books.append(book);

    const auto ref = scripture::parseReference(QStringLiteral(u"John 3:16"), bible);
    QVERIFY(ref.valid);
    QCOMPARE(ref.book, 43);
    QCOMPARE(ref.chapter, 3);
    QCOMPARE(ref.verse, 16);
    QCOMPARE(scripture::referenceToString(ref, bible), QStringLiteral(u"John 3:16"));

    const auto bytes = scripture::encodeBibleFile(bible);
    const auto loaded = scripture::decodeBibleFile(bytes);
    QVERIFY(loaded.ok);
    QCOMPARE(loaded.bible.books.size(), 1);
}

void ProSlidesTest::timerLifecycle()
{
    ps::timers::TimerManager manager;
    const QString id = manager.startTimer(QStringLiteral(u"Pre-service"), 1, 0);
    QVERIFY(!id.isEmpty());
    QCOMPARE(manager.activeCount(), 1);
    QVERIFY(manager.pauseTimer(id));
    QCOMPARE(manager.activeCount(), 0);
    QVERIFY(manager.resumeTimer(id));
    QCOMPARE(manager.activeCount(), 1);
    QVERIFY(manager.stopTimer(id));
    QCOMPARE(manager.timers().size(), 0);
}

void ProSlidesTest::templateRoundTrip()
{
    ps::templates::Template t;
    t.id = makeId();
    t.name = QStringLiteral(u"Lyric Dark");
    t.category = QStringLiteral(u"Text");
    t.color = QStringLiteral(u"#3ddc84");
    t.settings.insert(QStringLiteral(u"backgroundColor"), QStringLiteral(u"#000000"));

    const auto obj = t.toJson();
    const auto loaded = ps::templates::Template::fromJson(obj);
    QCOMPARE(loaded.name, t.name);
    QCOMPARE(loaded.settings.value(QStringLiteral(u"backgroundColor")).toString(), QStringLiteral(u"#000000"));
}

void ProSlidesTest::overlaySceneSwitch()
{
    overlays::OverlayManager manager;
    overlays::Overlay o1;
    o1.id = QStringLiteral(u"o1");
    o1.name = QStringLiteral(u"Lower Third");
    overlays::Overlay o2;
    o2.id = QStringLiteral(u"o2");
    o2.name = QStringLiteral(u"QR Code");

    overlays::Scene scene;
    scene.id = QStringLiteral(u"s1");
    scene.name = QStringLiteral(u"Welcome");
    scene.overlays = { QStringLiteral(u"o1"), QStringLiteral(u"o2")};
    scene.hiddenOverlays = { QStringLiteral(u"o2")};

    // exercise JSON round trip
    QCOMPARE(overlays::Overlay::fromJson(o1.toJson()).name, QStringLiteral(u"Lower Third"));
    QCOMPARE(overlays::Scene::fromJson(scene.toJson()).overlays.size(), size_t(2));
}

void ProSlidesTest::triggerMapping()
{
    ps::triggers::TriggerManager manager;
    manager.addMidiTrigger(QStringLiteral(u"next_slide"), 60);
    manager.addKeyTrigger(QStringLiteral(u"clear_all"), QStringLiteral(u"PgDown"));

    QSignalSpy spy(&manager, SIGNAL(actionRequested(const QString&, const QJsonObject&)));
    manager.handleMidi(0x90, 0, 60, 100);
    manager.handleMidi(0x90, 0, 60, 0);   // note-off ignored
    manager.handleMidi(0x90, 0, 61, 100); // wrong note
    QCOMPARE(spy.count(), 1);

    manager.triggerKey(QStringLiteral(u"pgdown"));
    QCOMPARE(spy.count(), 2);

    const auto json = ps::triggers::Trigger::fromJson(
        ps::triggers::Trigger::fromJson(
            QJsonObject{{QStringLiteral(u"action"), QStringLiteral(u"next_slide")}, {QStringLiteral(u"note"), 60}}).toJson());
    QCOMPARE(json.note, 60);
}

void ProSlidesTest::backupSnapshot()
{
    const QString dir = QDir::temp().filePath(QStringLiteral(u"proslides-backup-test-")+ makeId());
    QDir().mkpath(QDir(dir).filePath(QStringLiteral(u"Shows")));
    ps::backup::BackupManager manager;
    manager.setDataDirectory(dir);
    const QString id = manager.backupNow(QStringLiteral(u"Unit test"));
    QVERIFY(!id.isEmpty());
    QVERIFY(QFile::exists(QDir(dir).filePath(QStringLiteral(u"Backups/")+ id + QStringLiteral(u"/manifest.json"))));
    QVERIFY(manager.backups().size() >= 1);
    QDir(dir).removeRecursively();
}

void ProSlidesTest::chordTranspose()
{
    QCOMPARE(ps::chords::transpose(QStringLiteral(u"G"), 2), QStringLiteral(u"A"));
    QCOMPARE(ps::chords::transpose(QStringLiteral(u"G/B"), 2), QStringLiteral(u"A/C#"));
    QCOMPARE(ps::chords::transpose(QStringLiteral(u"C#m7"), -1), QStringLiteral(u"Cm7"));
    QCOMPARE(ps::chords::transpose(QStringLiteral(u"Bb"), 2, true), QStringLiteral(u"C"));

    Show show;
    show.id = makeId();
    Slide slide;
    slide.id = makeId();
    Item item;
    Line line;
    Chord chord;
    chord.id = makeId();
    chord.key = QStringLiteral(u"G");
    line.chords.append(chord);
    item.lines.append(line);
    slide.items.append(item);
    show.slides.insert(slide.id, slide);

    ps::chords::transposeShow(show, 2);
    QCOMPARE(show.slides.begin()->second.items.at(0).lines.at(0).chords.at(0).key, QStringLiteral(u"A"));
    QCOMPARE(ps::chords::detectKey(show), QStringLiteral(u"A"));
}

QTEST_MAIN(ProSlidesTest)
#include "tst_proslides.moc"
