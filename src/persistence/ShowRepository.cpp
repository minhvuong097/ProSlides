#include "ShowRepository.h"
#include "Database.h"
#include "ShowFile.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>

namespace ps {
namespace {

QString jsonString(const QJsonObject& o)
{
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

void bindOrNull(QSqlQuery& q, const QString& name, const std::optional<QString>& v)
{
    if (v) q.bindValue(name, *v);
    else q.bindValue(name, QVariant{});
}

QString normalizeVietnamese(QString s)
{
    QString nfd = s.normalized(QString::NormalizationForm_D).toLower();
    nfd.replace(u'\u0111', u'd'); // đ -> d
    QString out;
    out.reserve(nfd.size());
    for (int i = 0; i < nfd.size(); ++i) {
        const QChar c = nfd[i];
        if (c.category() != QChar::Mark_NonSpacing &&
            c.category() != QChar::Mark_SpacingCombining)
            out += c;
    }
    return out;
}

} // namespace

ShowRepository::ShowRepository(Database& db) : m_db(db)
{
}

QString ShowRepository::ftsQuery(const QString& userText)
{
    const QString normalized = normalizeVietnamese(userText);
    const auto words = normalized.split(QRegularExpression(u"\\s+"), Qt::SkipEmptyParts);
    QStringList clauses;
    for (const auto& w : words) {
        QString clean;
        for (const QChar c : w) {
            if (c.isLetterOrNumber() || c == u'_') clean += c;
        }
        if (!clean.isEmpty()) clauses << u'"' + clean + u"\"*";
    }
    return clauses.join(u' ');
}

bool ShowRepository::indexShow(const Show& show, const QString& filePath)
{
    DbTransaction tx(m_db);
    if (!tx.ok()) return false;

    QSqlQuery q(m_db.database());
    q.prepare(u"INSERT INTO shows (id,name,file_path,origin,category,is_private,locked,"
              u"created,modified,used,tags,meta_key,json) "
              u"VALUES (:id,:name,:path,:origin,:category,:private,:locked,"
              u":created,:modified,:used,:tags,:metaKey,:json) "
              u"ON CONFLICT(id) DO UPDATE SET "
              u"name=excluded.name, file_path=excluded.file_path, origin=excluded.origin,"
              u"category=excluded.category, is_private=excluded.is_private,"
              u"locked=excluded.locked, created=excluded.created, modified=excluded.modified,"
              u"used=excluded.used, tags=excluded.tags, meta_key=excluded.meta_key,"
              u"json=excluded.json");
    q.bindValue(u":id", show.id);
    q.bindValue(u":name", show.name);
    q.bindValue(u":path", filePath);
    bindOrNull(q, u":origin", show.origin);
    bindOrNull(q, u":category", show.category);
    q.bindValue(u":private", show.isPrivate ? 1 : 0);
    q.bindValue(u":locked", show.locked ? 1 : 0);
    q.bindValue(u":created", show.timestamps.created);
    if (show.timestamps.modified) q.bindValue(u":modified", *show.timestamps.modified);
    else q.bindValue(u":modified", QVariant{});
    if (show.timestamps.used) q.bindValue(u":used", *show.timestamps.used);
    else q.bindValue(u":used", QVariant{});
    q.bindValue(u":tags", show.tags.join(u", "));
    bindOrNull(q, u":metaKey", show.meta.key);
    q.bindValue(u":json", jsonString(show.toJson()));
    if (!q.exec()) return false;

    // Replace normalized children (cascades clear old rows).
    for (const char* table : { "slides", "layouts", "media" }) {
        QSqlQuery del(m_db.database());
        del.prepare(QString::fromUtf8("DELETE FROM %1 WHERE show_id = :sid").arg(table));
        del.bindValue(u":sid", show.id);
        del.exec();
    }

    indexSlides(show);
    indexLayouts(show);
    indexMedia(show);
    return true;
}

void ShowRepository::indexSlides(const Show& show)
{
    int pos = 0;
    for (const auto& [id, slide] : show.slides) {
        QSqlQuery q(m_db.database());
        q.prepare(u"INSERT INTO slides (id,show_id,group_name,title,plain_text,notes,"
                  u"musical_key,color,background,position,locked,json) "
                  u"VALUES (:id,:show,:group,:title,:text,:notes,:key,:color,:bg,"
                  u":pos,:locked,:json)");
        q.bindValue(u":id", id);
        q.bindValue(u":show", show.id);
        bindOrNull(q, u":group", slide.group);
        q.bindValue(u":title", slide.plainText().section(u'\n', 0, 0));
        q.bindValue(u":text", slide.plainText());
        q.bindValue(u":notes", slide.notes);
        bindOrNull(q, u":key", slide.key);
        bindOrNull(q, u":color", slide.color);
        bindOrNull(q, u":bg", slide.settings.backgroundImage);
        q.bindValue(u":pos", pos++);
        q.bindValue(u":locked", slide.locked ? 1 : 0);
        q.bindValue(u":json", jsonString(slide.toJson()));
        q.exec();

        if (slide.timeline) indexTimeline(u"slide", id, slide.timeline);
    }
}

void ShowRepository::indexLayouts(const Show& show)
{
    int lpos = 0;
    for (const auto& [id, layout] : show.layouts) {
        QSqlQuery q(m_db.database());
        q.prepare(u"INSERT INTO layouts (id,show_id,name,notes,is_active,position,json) "
                  u"VALUES (:id,:show,:name,:notes,:active,:pos,:json)");
        q.bindValue(u":id", id);
        q.bindValue(u":show", show.id);
        q.bindValue(u":name", layout.name);
        q.bindValue(u":notes", layout.notes);
        q.bindValue(u":active", (id == show.settings.activeLayout) ? 1 : 0);
        q.bindValue(u":pos", lpos++);
        q.bindValue(u":json", jsonString(layout.toJson()));
        q.exec();

        int spos = 0;
        for (const auto& ref : layout.slides) {
            QSqlQuery r(m_db.database());
            r.prepare(u"INSERT OR REPLACE INTO layout_slides "
                      u"(layout_id,slide_id,position,disabled,transition,background,duration_ms) "
                      u"VALUES (:layout,:slide,:pos,:disabled,:transition,:bg,:duration)");
            r.bindValue(u":layout", id);
            r.bindValue(u":slide", ref.id);
            r.bindValue(u":pos", spos++);
            r.bindValue(u":disabled", ref.disabled ? 1 : 0);
            if (ref.transition) r.bindValue(u":transition", transitionTypeToString(ref.transition->type));
            else r.bindValue(u":transition", QVariant{});
            bindOrNull(r, u":bg", ref.background);
            if (ref.timer) r.bindValue(u":duration", *ref.timer);
            else r.bindValue(u":duration", QVariant{});
            r.exec();
        }
        if (layout.timeline) indexTimeline(u"layout", id, layout.timeline);
    }
}

void ShowRepository::indexMedia(const Show& show)
{
    for (const auto& [id, m] : show.media) {
        QSqlQuery q(m_db.database());
        q.prepare(u"INSERT INTO media (id,show_id,name,path,type,video_type,muted,loop,json) "
                  u"VALUES (:id,:show,:name,:path,:type,:videoType,:muted,:loop,:json)");
        q.bindValue(u":id", id);
        q.bindValue(u":show", show.id);
        q.bindValue(u":name", m.name);
        q.bindValue(u":path", m.path);
        q.bindValue(u":type", mediaTypeToString(m.type));
        bindOrNull(q, u":videoType", m.videoType);
        q.bindValue(u":muted", m.muted ? 1 : 0);
        q.bindValue(u":loop", m.loop ? 1 : 0);
        q.bindValue(u":json", jsonString(m.toJson()));
        q.exec();
    }
}

void ShowRepository::indexTimeline(const QString& ownerType, const Id& ownerId,
                                   const std::optional<QJsonObject>& obj)
{
    if (!obj) return;
    const Timeline timeline = Timeline::fromJson(*obj);
    for (const auto& action : timeline.actions) {
        QSqlQuery q(m_db.database());
        q.prepare(u"INSERT OR REPLACE INTO timeline_actions "
                  u"(id,owner_type,owner_id,time_ms,duration_s,name,type,color,json) "
                  u"VALUES (:id,:ownerType,:ownerId,:time,:duration,:name,:type,:color,:json)");
        q.bindValue(u":id", action.id);
        q.bindValue(u":ownerType", ownerType);
        q.bindValue(u":ownerId", ownerId);
        q.bindValue(u":time", action.time);
        q.bindValue(u":duration", action.duration);
        q.bindValue(u":name", action.name);
        q.bindValue(u":type", action.type);
        bindOrNull(q, u":color", action.color);
        q.bindValue(u":json", jsonString(action.toJson()));
        q.exec();
    }
}

bool ShowRepository::saveShow(const Show& show, QString* error)
{
    const QString filePath = showFilePath(m_dataDir, show.name.isEmpty() ? show.id : show.name);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = file.errorString();
        return false;
    }
    const auto bytes = encodeShowFile(show);
    if (file.write(bytes) != bytes.size()) {
        if (error) *error = u"Failed to write .show file";
        return false;
    }
    file.close();

    if (!indexShow(show, filePath)) {
        if (error) *error = m_db.lastError();
        return false;
    }
    return true;
}

std::optional<Show> ShowRepository::loadShow(const Id& id, QString* error) const
{
    QSqlQuery q(m_db.database());
    q.prepare(u"SELECT json FROM shows WHERE id = :id");
    q.bindValue(u":id", id);
    if (!q.exec() || !q.next()) {
        if (error) *error = u"Show not found: " + id;
        return std::nullopt;
    }
    const auto obj = QJsonDocument::fromJson(q.value(0).toByteArray()).object();
    Show show = Show::fromJson(obj);
    show.id = id;
    return show;
}

bool ShowRepository::removeShow(const Id& id)
{
    QSqlQuery q(m_db.database());
    q.prepare(u"DELETE FROM shows WHERE id = :id");
    q.bindValue(u":id", id);
    return q.exec();
}

QVector<ShowSummary> ShowRepository::listShows() const
{
    QVector<ShowSummary> out;
    QSqlQuery q(m_db.database());
    q.exec(u"SELECT s.id,s.name,s.category,s.file_path,s.created,s.modified,"
           u"(SELECT COUNT(*) FROM slides WHERE show_id=s.id) "
           u"FROM shows s ORDER BY s.modified DESC, s.name COLLATE NOCASE");
    while (q.next()) {
        ShowSummary s;
        s.id = q.value(0).toString();
        s.name = q.value(1).toString();
        s.category = q.value(2).toString();
        s.filePath = q.value(3).toString();
        s.created = q.value(4).toLongLong();
        s.modified = q.value(5).toLongLong();
        s.slideCount = q.value(6).toInt();
        out << s;
    }
    return out;
}

QVector<SearchHit> ShowRepository::search(const QString& query, int limit) const
{
    QVector<SearchHit> out;
    const QString fts = ftsQuery(query);
    if (fts.isEmpty()) return out;

    QSqlQuery q(m_db.database());
    q.prepare(u"SELECT sl.show_id, sl.id, sh.name, sl.group_name, sl.title, "
              u"snippet(slides_fts, 2, '<mark>', '</mark>', '…', 18), "
              u"bm25(slides_fts, 4.0, 2.0, 1.0, 0.5, 0.5) AS rank "
              u"FROM slides_fts "
              u"JOIN slides sl ON sl.rowid = slides_fts.rowid "
              u"JOIN shows sh ON sh.id = sl.show_id "
              u"WHERE slides_fts MATCH :q ORDER BY rank LIMIT :limit");
    q.bindValue(u":q", fts);
    q.bindValue(u":limit", limit);
    if (!q.exec()) return out;

    while (q.next()) {
        SearchHit hit;
        hit.showId = q.value(0).toString();
        hit.slideId = q.value(1).toString();
        hit.showName = q.value(2).toString();
        hit.group = q.value(3).toString();
        hit.title = q.value(4).toString();
        hit.snippet = q.value(5).toString();
        hit.rank = q.value(6).toDouble();
        out << hit;
    }
    return out;
}

bool ShowRepository::saveProject(const Project& project, QString* error)
{
    DbTransaction tx(m_db);
    if (!tx.ok()) return false;
    QSqlQuery q(m_db.database());
    q.prepare(u"INSERT INTO projects (id,name,notes,parent,created,modified,used,archived,json) "
              u"VALUES (:id,:name,:notes,:parent,:created,:modified,:used,:archived,:json) "
              u"ON CONFLICT(id) DO UPDATE SET name=excluded.name, notes=excluded.notes,"
              u"parent=excluded.parent, modified=excluded.modified, used=excluded.used,"
              u"archived=excluded.archived, json=excluded.json");
    q.bindValue(u":id", project.id);
    q.bindValue(u":name", project.name);
    q.bindValue(u":notes", project.notes);
    q.bindValue(u":parent", project.parent);
    q.bindValue(u":created", project.created);
    if (project.modified) q.bindValue(u":modified", *project.modified);
    if (project.used) q.bindValue(u":used", *project.used);
    q.bindValue(u":archived", project.archived ? 1 : 0);
    q.bindValue(u":json", jsonString(project.toJson()));
    if (!q.exec()) { if (error) *error = m_db.lastError(); return false; }

    QSqlQuery del(m_db.database());
    del.prepare(u"DELETE FROM project_items WHERE project_id = :id");
    del.bindValue(u":id", project.id);
    del.exec();

    int pos = 0;
    for (const auto& ref : project.shows) {
        QSqlQuery i(m_db.database());
        i.prepare(u"INSERT INTO project_items "
                  u"(project_id,position,show_id,layout_id,section,name,item_type,color,notes,played,json) "
                  u"VALUES (:pid,:pos,:sid,:layout,:section,:name,:type,:color,:notes,:played,:json)");
        i.bindValue(u":pid", project.id);
        i.bindValue(u":pos", pos++);
        i.bindValue(u":sid", ref.id);
        bindOrNull(i, u":layout", ref.layout);
        i.bindValue(u":section", ref.type ? showTypeToString(*ref.type) : QVariant{});
        bindOrNull(i, u":name", ref.name);
        bindOrNull(i, u":color", ref.color);
        bindOrNull(i, u":notes", ref.notes);
        i.bindValue(u":played", ref.played ? 1 : 0);
        i.bindValue(u":json", jsonString(ref.toJson()));
        i.exec();
    }
    return true;
}

std::optional<Project> ShowRepository::loadProject(const Id& id) const
{
    QSqlQuery q(m_db.database());
    q.prepare(u"SELECT json FROM projects WHERE id = :id");
    q.bindValue(u":id", id);
    if (!q.exec() || !q.next()) return std::nullopt;
    Project p = Project::fromJson(QJsonDocument::fromJson(q.value(0).toByteArray()).object());
    p.id = id;
    return p;
}

QVector<Project> ShowRepository::listProjects() const
{
    QVector<Project> out;
    QSqlQuery q(m_db.database());
    q.exec(u"SELECT json FROM projects WHERE archived = 0 ORDER BY name COLLATE NOCASE");
    while (q.next())
        out << Project::fromJson(QJsonDocument::fromJson(q.value(0).toByteArray()).object());
    return out;
}

bool ShowRepository::removeProject(const Id& id)
{
    QSqlQuery q(m_db.database());
    q.prepare(u"DELETE FROM projects WHERE id = :id");
    q.bindValue(u":id", id);
    return q.exec();
}

int ShowRepository::reindexFromDisk(QString* error)
{
    const QDir dir(QDir(m_dataDir).filePath(u"Shows"));
    const auto files = dir.entryList({u"*.show"}, QDir::Files);
    int count = 0;
    for (const auto& name : files) {
        QFile f(dir.filePath(name));
        if (!f.open(QIODevice::ReadOnly)) continue;
        const auto result = decodeShowFileRepair(f.readAll());
        f.close();
        if (!result.ok) {
            if (error) *error = name + u": " + result.error;
            continue;
        }
        Show show = result.show;
        if (show.id.isEmpty()) show.id = QFileInfo(name).completeBaseName();
        if (show.name.isEmpty()) show.name = QFileInfo(name).completeBaseName();
        if (indexShow(show, dir.filePath(name))) ++count;
    }
    return count;
}

} // namespace ps
