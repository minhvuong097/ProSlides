#include "PresentationController.h"

#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMetaObject>

namespace ps::presentation {

PresentationController::PresentationController(QObject* parent)
    : QObject(parent)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/ProSlides";
    QDir().mkpath(base);
    QDir().mkpath(QDir(base).filePath(u"Shows"));
    QDir().mkpath(QDir(base).filePath(u"Media"));

    QString dbError;
    if (!m_database.open(&dbError)) emit errorOccurred(dbError);
    m_repository.setDataDirectory(base);

    connect(&m_document, &ShowDocument::changed, this, &PresentationController::onDocumentChanged);
    connect(&m_output, &OutputManager::stateChanged, this, &PresentationController::stateChanged);
    connect(&m_output, &OutputManager::slidesChanged, this, &PresentationController::slidesChanged);
    connect(&m_remote, &remote::RemoteServer::apiRequested,
            this, &PresentationController::onApiRequested);
    connect(&m_remote, &remote::RemoteServer::errorOccurred,
            this, &PresentationController::errorOccurred);

    m_sync.setTransport(&m_remote);
    connect(&m_discovery, &remote::RemoteDiscovery::peerDiscovered,
            this, [this](const remote::RemotePeer& peer) {
                emit toast(u"Found remote: " + peer.name + u" (" + peer.host + u")");
            });

    m_clock.setInterval(1000);
    connect(&m_clock, &QTimer::timeout, this, &PresentationController::updateClock);
    m_clock.start();
    updateClock();

    newShow(u"Untitled Service");
}

// ---------------------------------------------------------------------------
// properties
// ---------------------------------------------------------------------------
QString PresentationController::activeShowName() const { return m_document.name(); }
int PresentationController::currentIndex() const { return m_output.currentIndex(); }
int PresentationController::slideCount() const { return m_output.slideCount(); }
bool PresentationController::remoteEnabled() const { return m_remote.isRunning(); }
bool PresentationController::modified() const { return m_document.modified(); }
QString PresentationController::clockText() const { return m_clockText; }

void PresentationController::updateClock()
{
    m_clockText = QDateTime::currentDateTime().toString(u"hh:mm");
    emit tick();
}

// ---------------------------------------------------------------------------
// show / document
// ---------------------------------------------------------------------------
void PresentationController::newShow(const QString& name)
{
    Show show;
    show.id = makeId();
    show.name = name.isEmpty() ? u"Untitled Service" : name;
    show.timestamps.created = nowMillis();
    const Id layoutId = makeId();
    Layout layout;
    layout.id = layoutId;
    layout.name = u"Main";
    show.layouts.insert(layoutId, layout);
    show.settings.activeLayout = layoutId;

    m_document.setShow(std::move(show));
    rebuildSlides();
    emit showChanged();
}

bool PresentationController::openShow(const QString& id)
{
    auto show = m_repository.loadShow(id);
    if (!show) {
        emit errorOccurred(u"Cannot open show: " + id);
        return false;
    }
    m_document.setShow(std::move(*show));
    rebuildSlides();
    emit showChanged();
    emit toast(u"Opened " + m_document.name());
    return true;
}

bool PresentationController::saveShow()
{
    QString error;
    if (!m_repository.saveShow(m_document.show(), &error)) {
        emit errorOccurred(error);
        return false;
    }
    m_document.markSaved();
    syncShowToCrdt();
    m_remote.publishShows(showsForRemote());
    emit showChanged();
    emit toast(u"Saved " + m_document.name());
    return true;
}

bool PresentationController::importShowFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(file.errorString());
        return false;
    }
    auto result = decodeShowFileRepair(file.readAll());
    file.close();
    if (!result.ok) {
        emit errorOccurred(result.error);
        return false;
    }
    m_document.setShow(std::move(result.show));
    rebuildSlides();
    emit showChanged();
    saveShow();
    return true;
}

void PresentationController::onDocumentChanged()
{
    rebuildSlides();
    emit showChanged();
}

void PresentationController::syncShowToCrdt()
{
    const Show& show = m_document.show();
    m_crdt.set({ u"shows", show.id, u"name" }, show.name);
    for (const auto& [id, slide] : show.slides) {
        QJsonObject payload;
        payload.insert(u"id", id);
        payload.insert(u"group", slide.group.value_or(QString{}));
        payload.insert(u"text", slide.plainText());
        payload.insert(u"notes", slide.notes);
        m_crdt.set({ u"shows", show.id, u"slides", id }, payload);
    }
}

// ---------------------------------------------------------------------------
// slides
// ---------------------------------------------------------------------------
QVariantMap PresentationController::slideToMap(const Id& id, int index) const
{
    QVariantMap m;
    const auto it = m_document.show().slides.constFind(id);
    m.insert(u"id", id);
    m.insert(u"index", index);
    if (it != m_document.show().slides.constEnd()) {
        m.insert(u"group", it->group.value_or(QString{}));
        m.insert(u"title", it->plainText().section(u'\n', 0, 0));
        m.insert(u"text", it->plainText());
        m.insert(u"notes", it->notes);
        m.insert(u"key", it->key.value_or(QString{}));
    }
    return m;
}

void PresentationController::rebuildSlides()
{
    m_slides.clear();
    m_slideOrder.clear();
    const Show& show = m_document.show();
    const auto lit = show.layouts.constFind(show.settings.activeLayout);
    if (lit == show.layouts.constEnd()) {
        emit slidesChanged();
        return;
    }
    int index = 0;
    for (const auto& ref : lit->second.slides) {
        if (ref.disabled || ref.remove) continue;
        if (!show.slides.contains(ref.id)) continue;
        m_slideOrder.append(ref.id);
        m_slides.append(slideToMap(ref.id, index++));
    }
    emit slidesChanged();
}

void PresentationController::pushOutput()
{
    QVector<QJsonObject> slides;
    for (const auto& id : m_slideOrder) {
        const auto it = m_document.show().slides.constFind(id);
        if (it == m_document.show().slides.constEnd()) continue;
        slides.append(it->toJson());
    }
    m_output.setSlides(slides);
}

QString PresentationController::addLyricSlide(const QString& group, const QString& text,
                                              const QString& key)
{
    const Id id = m_document.addLyricSlide(group, text, key);
    if (id.isEmpty()) emit errorOccurred(u"Could not create slide");
    pushOutput();
    return id;
}

QString PresentationController::addMediaSlide(const QString& group, const QString& path)
{
    const Id id = m_document.addMediaSlide(group, path);
    if (id.isEmpty()) emit errorOccurred(u"Could not create media slide");
    pushOutput();
    return id;
}

bool PresentationController::deleteSlide(const QString& id)
{
    const bool ok = m_document.removeSlide(id);
    if (ok) pushOutput();
    return ok;
}

bool PresentationController::duplicateSlide(const QString& id)
{
    const Id newId = m_document.duplicateSlide(id);
    if (newId.isEmpty()) return false;
    pushOutput();
    return true;
}

void PresentationController::moveSlide(int from, int to)
{
    if (from == to || from < 0 || to < 0) return;
    const int delta = to > from ? 1 : -1;
    const int steps = std::abs(to - from);
    for (int i = 0; i < steps; ++i) {
        const Id id = m_slideOrder.value(from);
        if (id.isEmpty()) break;
        m_document.moveSlide(id, delta);
    }
    pushOutput();
}

void PresentationController::nextSlide()    { m_output.next(); }
void PresentationController::previousSlide(){ m_output.previous(); }
void PresentationController::goToSlide(int index) { m_output.goTo(index); }
void PresentationController::clearOutput() { m_output.clearAll(); }
void PresentationController::restoreOutput() { m_output.restore(); }

void PresentationController::startPresenting()
{
    pushOutput();
    if (m_output.displayCount() == 0 && m_output.slideCount() > 0) openDisplay(0);
    m_remote.publishOutput(QJsonObject{{u"index", m_output.currentIndex()}});
    emit toast(u"Presenting");
}

void PresentationController::stopPresenting()
{
    closeDisplays();
    m_output.clearAll();
    emit toast(u"Presentation stopped");
}

// ---------------------------------------------------------------------------
// search
// ---------------------------------------------------------------------------
void PresentationController::search(const QString& query)
{
    m_searchResults.clear();
    const auto hits = m_repository.search(query, 60);
    for (const auto& hit : hits) {
        QVariantMap m;
        m.insert(u"showId", hit.showId);
        m.insert(u"slideId", hit.slideId);
        m.insert(u"showName", hit.showName);
        m.insert(u"group", hit.group);
        m.insert(u"title", hit.title);
        m.insert(u"snippet", hit.snippet);
        m.insert(u"rank", hit.rank);
        m_searchResults.append(m);
    }
    emit searchResultsChanged();
}

bool PresentationController::openSearchResult(int row)
{
    if (row < 0 || row >= m_searchResults.size()) return false;
    const auto m = m_searchResults.at(row).toMap();
    if (!openShow(m.value(u"showId").toString())) return false;
    const QString slideId = m.value(u"slideId").toString();
    const int index = m_slideOrder.indexOf(slideId);
    if (index >= 0) goToSlide(index);
    return true;
}

// ---------------------------------------------------------------------------
// remote / displays / media
// ---------------------------------------------------------------------------
bool PresentationController::startRemote(const QString& password, quint16 port)
{
    if (!m_remote.start(port, password)) return false;
    m_discovery.start(port, u"ProSlides-" + QHostInfo::localHostName());

    QString ip = u"127.0.0.1";
    for (const auto& address : QNetworkInterface::allAddresses()) {
        if (address.isIPv4() && !address.isLoopback()) { ip = address.toString(); break; }
    }
    m_remoteAddress = u"http://" + ip + u":" + QString::number(port) + u"/remote";
    m_remote.publishShows(showsForRemote());
    emit remoteChanged();
    emit toast(u"Remote ready: " + m_remoteAddress);
    return true;
}

void PresentationController::stopRemote()
{
    m_remote.stop();
    m_discovery.stop();
    m_remoteAddress.clear();
    emit remoteChanged();
}

void PresentationController::attachEngine(QQmlEngine* engine)
{
    m_engine = engine;
    m_output.attachEngine(engine);
}

bool PresentationController::openDisplay(int screenIndex)
{
    if (!m_engine) {
        emit errorOccurred(u"QML engine not attached");
        return false;
    }
    return m_output.addDisplay(screenIndex);
}

void PresentationController::closeDisplays()
{
    m_output.removeAllDisplays();
}

void PresentationController::scanMedia(const QString& directory)
{
    m_mediaLibrary.setRoot(directory);
    m_mediaLibrary.scan();
    m_mediaFiles.clear();
    for (const auto& file : m_mediaLibrary.files()) {
        QVariantMap item;
        item.insert(u"name", file.name);
        item.insert(u"path", file.path);
        item.insert(u"kind", mediaTypeToString(file.type));
        m_mediaFiles.append(item);
    }
    emit mediaFilesChanged();
    emit toast(u"Media scanned: " + QString::number(m_mediaLibrary.files().size()) + u" files");
}

QJsonArray PresentationController::showsForRemote() const
{
    QJsonArray out;
    for (const auto& summary : m_repository.listShows()) {
        QJsonObject o;
        o.insert(u"id", summary.id);
        o.insert(u"name", summary.name);
        o.insert(u"category", summary.category);
        o.insert(u"slides", summary.slideCount);
        out.append(o);
    }
    return out;
}

QVariantMap PresentationController::servicePlan() const
{
    QVariantMap out;
    out.insert(u"name", u"Service Flow");
    out.insert(u"index", m_serviceIndex);
    if (m_activeProjectId.isEmpty()) return out;
    auto project = m_repository.loadProject(m_activeProjectId);
    if (!project) return out;
    out.insert(u"name", project->name);
    QVariantList items;
    for (const auto& ref : project->shows) {
        QVariantMap item;
        item.insert(u"id", ref.id);
        item.insert(u"name", ref.name.value_or(ref.id));
        item.insert(u"played", ref.played);
        item.insert(u"color", ref.color.value_or(QString{}));
        items.append(item);
    }
    out.insert(u"items", items);
    return out;
}

bool PresentationController::openProject(const QString& id)
{
    if (!m_repository.loadProject(id)) {
        emit errorOccurred(u"Project not found");
        return false;
    }
    m_activeProjectId = id;
    m_serviceIndex = -1;
    emit stateChanged();
    return true;
}

void PresentationController::nextServiceItem()
{
    auto project = m_repository.loadProject(m_activeProjectId);
    if (!project || project->shows.isEmpty()) return;
    m_serviceIndex = qMin(m_serviceIndex + 1, int(project->shows.size()) - 1);
    openShow(project->shows.at(m_serviceIndex).id);
    goToSlide(0);
    emit stateChanged();
}

// ---------------------------------------------------------------------------
// API bridge (phone remote)
// ---------------------------------------------------------------------------
void PresentationController::onApiRequested(const QString& api, const QJsonObject& data,
                                            const QString& clientId, bool wantsResponse)
{
    QJsonValue result = QJsonValue::Null;

    if (api == remote::Api::NextSlide) { nextSlide(); result = m_output.currentIndex(); }
    else if (api == remote::Api::PreviousSlide) { previousSlide(); result = m_output.currentIndex(); }
    else if (api == remote::Api::IndexSelectSlide) { goToSlide(data.value(u"index").toInt()); result = m_output.currentIndex(); }
    else if (api == remote::Api::ClearAll) { clearOutput(); result = true; }
    else if (api == remote::Api::RestoreOutput) { restoreOutput(); result = true; }
    else if (api == remote::Api::ClearSlide) { m_output.clearSlide(); result = true; }
    else if (api == remote::Api::ClearBackground) { m_output.setBackground({}); result = true; }
    else if (api == remote::Api::GetOutput) {
        QJsonObject o;
        o.insert(u"index", m_output.currentIndex());
        o.insert(u"slide", m_output.currentSlide());
        o.insert(u"cleared", m_output.cleared());
        result = o;
    }
    else if (api == remote::Api::GetCleared) { result = m_output.cleared(); }
    else if (api == remote::Api::GetShows) { result = showsForRemote(); }
    else if (api == remote::Api::GetGroups) {
        QJsonArray groups;
        for (const auto& [id, slide] : m_document.show().slides)
            if (slide.group && !groups.contains(*slide.group)) groups.append(*slide.group);
        result = groups;
    }
    else if (api == remote::Api::GetSlide) {
        const QString id = data.value(u"slideId").toString();
        const auto it = m_document.show().slides.constFind(id);
        result = it != m_document.show().slides.constEnd() ? it->toJson() : QJsonValue::Null;
    }
    else if (api == remote::Api::StartShow) { openShow(data.value(u"id").toString()); result = true; }
    else if (api == remote::Api::ChangeLayout) { setLayoutFromRemote(data.value(u"layoutId").toString()); result = true; }
    else if (api == "get_show_layout") {
        QJsonArray arr;
        for (const auto& [id, layout] : m_document.show().layouts) {
            QJsonObject o = layout.toJson();
            o.insert(u"id", id);
            arr.append(o);
        }
        result = arr;
    }
    else if (api == "get_projects") {
        QJsonArray arr;
        for (const auto& p : m_repository.listProjects()) arr.append(p.toJson());
        result = arr;
    }
    else {
        emit toast(u"Unknown API action: " + api);
    }

    if (wantsResponse) m_remote.reply(clientId, api, result);
}

void PresentationController::setLayoutFromRemote(const QString& layoutId)
{
    if (m_document.setActiveLayout(layoutId)) {
        pushOutput();
        emit stateChanged();
    }
}

} // namespace ps::presentation
