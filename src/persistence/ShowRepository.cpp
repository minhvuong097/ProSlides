#include "persistence/ShowRepository.h"
#include "persistence/Database.h"
#include "core/ShowFile.h"

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

// QString() is a *null* string; Qt binds it as SQL NULL and breaks NOT NULL columns.
QString nn(const QString& s) { return s.isNull() ? QStringLiteral(u"") : s; }

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
    const auto words = normalized.split(QRegularExpression(QStringLiteral(u"\\s+")), Qt::SkipEmptyParts);
    QStringList clauses;
    for (const auto& w : words) {
        QString clean;
        for (const QChar c : w) {
            if (c.isLetterOrNumber() || c == u'_') clean += c;
        }
        if (!clean.isEmpty()) clauses << u'"' + clean + QStringLiteral(u"\"*");
    }
    return clauses.join(u' ');
}

bool ShowRepository::indexShow(const Show& show, const QString& filePath)
{
    DbTransaction tx(m_db);
    if (!tx.ok()) return false;

    QSqlQuery q(m_db.database());
    q.prepare(QStringLiteral(u"INSERT INTO shows (id,name,file_path,origin,category,is_private,locked,"
              u"created,modified,used,tags,meta_key,json) "
              u"VALUES (:id,:name,:path,:origin,:category,:private,:locked,"
              u":created,:modified,:used,:tags,:metaKey,:json) "
              u"ON CONFLICT(id) DO UPDATE SET "
              u"name=excluded.name, file_path=excluded.file_path, origin=excluded.origin,"
              u"category=excluded.category, is_private=excluded.is_private,"
              u"locked=excluded.locked, created=excluded.created, modified=excluded.modified,"
              u"used=excluded.used, tags=excluded.tags, meta_key=excluded.meta_key,"
              u"json=excluded.json"));
    q.bindValue(QStringLiteral(u":id"), show.id);
    q.bindValue(QStringLiteral(u":name"), nn(show.name));
    q.bindValue(QStringLiteral(u":path"), filePath);
    bindOrNull(q, QStringLiteral(u":origin"), show.origin);
    bindOrNull(q, QStringLiteral(u":category"), show.category);
    q.bindValue(QStringLiteral(u":private"), show.isPrivate ? 1 : 0);
    q.bindValue(QStringLiteral(u":locked"), show.locked ? 1 : 0);
    q.bindValue(QStringLiteral(u":created"), show.timestamps.created);
    if (show.timestamps.modified) q.bindValue(QStringLiteral(u":modified"), *show.timestamps.modified);
    else q.bindValue(QStringLiteral(u":modified"), QVariant{});
    if (show.timestamps.used) q.bindValue(QStringLiteral(u":used"), *show.timestamps.used);
    else q.bindValue(QStringLiteral(u":used"), QVariant{});
    q.bindValue(QStringLiteral(u":tags"), show.tags.join(QStringLiteral(u", ")));
    bindOrNull(q, QStringLiteral(u":metaKey"), show.meta.key);
    q.bindValue(QStringLiteral(u":json"), jsonString(show.toJson()));
    if (!q.exec()) return false;

    // Replace normalized children (cascades clear old rows).
    for (const char* table : { "slides", "layouts", "media" }) {
        QSqlQuery del(m_db.database());
        del.prepare(QString::fromUtf8("DELETE FROM %1 WHERE show_id = :sid").arg(table));
        del.bindValue(QStringLiteral(u":sid"), show.id);
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
        q.prepare(QStringLiteral(u"INSERT INTO slides (id,show_id,group_name,title,plain_text,notes,"
                  u"musical_key,color,background,position,locked,json) "
                  u"VALUES (:id,:show,:group,:title,:text,:notes,:key,:color,:bg,"
                  u":pos,:locked,:json)"));
        q.bindValue(QStringLiteral(u":id"), id);
        q.bindValue(QStringLiteral(u":show"), show.id);
        bindOrNull(q, QStringLiteral(u":group"), slide.group);
        q.bindValue(QStringLiteral(u":title"), slide.plainText().section(u'\n', 0, 0));
        q.bindValue(QStringLiteral(u":text"), nn(slide.plainText()));
        q.bindValue(QStringLiteral(u":notes"), nn(slide.notes));
        bindOrNull(q, QStringLiteral(u":key"), slide.key);
        bindOrNull(q, QStringLiteral(u":color"), slide.color);
        bindOrNull(q, QStringLiteral(u":bg"), slide.settings.backgroundImage);
        q.bindValue(QStringLiteral(u":pos"), pos++);
        q.bindValue(QStringLiteral(u":locked"), slide.locked ? 1 : 0);
        q.bindValue(QStringLiteral(u":json"), jsonString(slide.toJson()));
        q.exec();

        if (slide.timeline) indexTimeline(QStringLiteral(u"slide"), id, slide.timeline);
    }
}

void ShowRepository::indexLayouts(const Show& show)
{
    int lpos = 0;
    for (const auto& [id, layout] : show.layouts) {
        QSqlQuery q(m_db.database());
        q.prepare(QStringLiteral(u"INSERT INTO layouts (id,show_id,name,notes,is_active,position,json) "
                  u"VALUES (:id,:show,:name,:notes,:active,:pos,:json)"));
        q.bindValue(QStringLiteral(u":id"), id);
        q.bindValue(QStringLiteral(u":show"), show.id);
        q.bindValue(QStringLiteral(u":name"), nn(layout.name));
        q.bindValue(QStringLiteral(u":notes"), nn(layout.notes));
        q.bindValue(QStringLiteral(u":active"), (id == show.settings.activeLayout) ? 1 : 0);
        q.bindValue(QStringLiteral(u":pos"), lpos++);
        q.bindValue(QStringLiteral(u":json"), jsonString(layout.toJson()));
        q.exec();

        int spos = 0;
        for (const auto& ref : layout.slides) {
            QSqlQuery r(m_db.database());
            r.prepare(QStringLiteral(u"INSERT OR REPLACE INTO layout_slides "
                      u"(layout_id,slide_id,position,disabled,transition,background,duration_ms) "
                      u"VALUES (:layout,:slide,:pos,:disabled,:transition,:bg,:duration)"));
            r.bindValue(QStringLiteral(u":layout"), id);
            r.bindValue(QStringLiteral(u":slide"), ref.id);
            r.bindValue(QStringLiteral(u":pos"), spos++);
            r.bindValue(QStringLiteral(u":disabled"), ref.disabled ? 1 : 0);
            if (ref.transition) r.bindValue(QStringLiteral(u":transition"), transitionTypeToString(ref.transition->type));
            else r.bindValue(QStringLiteral(u":transition"), QVariant{});
            bindOrNull(r, QStringLiteral(u":bg"), ref.background);
            if (ref.timer) r.bindValue(QStringLiteral(u":duration"), *ref.timer);
            else r.bindValue(QStringLiteral(u":duration"), QVariant{});
            r.exec();
        }
        if (layout.timeline) indexTimeline(QStringLiteral(u"layout"), id, layout.timeline);
    }
}

void ShowRepository::indexMedia(const Show& show)
{
    for (const auto& [id, m] : show.media) {
        QSqlQuery q(m_db.database());
        q.prepare(QStringLiteral(u"INSERT INTO media (id,show_id,name,path,type,video_type,muted,loop,json) "
                  u"VALUES (:id,:show,:name,:path,:type,:videoType,:muted,:loop,:json)"));
        q.bindValue(QStringLiteral(u":id"), id);
        q.bindValue(QStringLiteral(u":show"), show.id);
        q.bindValue(QStringLiteral(u":name"), nn(m.name));
        q.bindValue(QStringLiteral(u":path"), m.path);
        q.bindValue(QStringLiteral(u":type"), mediaTypeToString(m.type));
        bindOrNull(q, QStringLiteral(u":videoType"), m.videoType);
        q.bindValue(QStringLiteral(u":muted"), m.muted ? 1 : 0);
        q.bindValue(QStringLiteral(u":loop"), m.loop ? 1 : 0);
        q.bindValue(QStringLiteral(u":json"), jsonString(m.toJson()));
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
        q.prepare(QStringLiteral(u"INSERT OR REPLACE INTO timeline_actions "
                  u"(id,owner_type,owner_id,time_ms,duration_s,name,type,color,json) "
                  u"VALUES (:id,:ownerType,:ownerId,:time,:duration,:name,:type,:color,:json)"));
        q.bindValue(QStringLiteral(u":id"), action.id);
        q.bindValue(QStringLiteral(u":ownerType"), ownerType);
        q.bindValue(QStringLiteral(u":ownerId"), ownerId);
        q.bindValue(QStringLiteral(u":time"), action.time);
        q.bindValue(QStringLiteral(u":duration"), action.duration);
        q.bindValue(QStringLiteral(u":name"), nn(action.name));
        q.bindValue(QStringLiteral(u":type"), action.type);
        bindOrNull(q, QStringLiteral(u":color"), action.color);
        q.bindValue(QStringLiteral(u":json"), jsonString(action.toJson()));
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
        if (error) *error = QStringLiteral(u"Failed to write .show file");
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
    q.prepare(QStringLiteral(u"SELECT json FROM shows WHERE id = :id"));
    q.bindValue(QStringLiteral(u":id"), id);
    if (!q.exec() || !q.next()) {
        if (error) *error = QStringLiteral(u"Show not found: ")+ id;
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
    q.prepare(QStringLiteral(u"DELETE FROM shows WHERE id = :id"));
    q.bindValue(QStringLiteral(u":id"), id);
    return q.exec();
}

QVector<ShowSummary> ShowRepository::listShows() const
{
    QVector<ShowSummary> out;
    QSqlQuery q(m_db.database());
    q.exec(QStringLiteral(u"SELECT s.id,s.name,s.category,s.file_path,s.created,s.modified,"
           u"(SELECT COUNT(*) FROM slides WHERE show_id=s.id) "
           u"FROM shows s ORDER BY s.modified DESC, s.name COLLATE NOCASE"));
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
    q.prepare(QStringLiteral(u"SELECT sl.show_id, sl.id, sh.name, sl.group_name, sl.title, "
              u"snippet(slides_fts, 2, '<mark>', '</mark>', '…', 18), "
              u"bm25(slides_fts, 4.0, 2.0, 1.0, 0.5, 0.5) AS rank "
              u"FROM slides_fts "
              u"JOIN slides sl ON sl.rowid = slides_fts.rowid "
              u"JOIN shows sh ON sh.id = sl.show_id "
              u"WHERE slides_fts MATCH :q ORDER BY rank LIMIT :limit"));
    q.bindValue(QStringLiteral(u":q"), fts);
    q.bindValue(QStringLiteral(u":limit"), limit);
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
    q.prepare(QStringLiteral(u"INSERT INTO projects (id,name,notes,parent,created,modified,used,archived,json) "
              u"VALUES (:id,:name,:notes,:parent,:created,:modified,:used,:archived,:json) "
              u"ON CONFLICT(id) DO UPDATE SET name=excluded.name, notes=excluded.notes,"
              u"parent=excluded.parent, modified=excluded.modified, used=excluded.used,"
              u"archived=excluded.archived, json=excluded.json"));
    q.bindValue(QStringLiteral(u":id"), project.id);
    q.bindValue(QStringLiteral(u":name"), nn(project.name));
    q.bindValue(QStringLiteral(u":notes"), nn(project.notes));
    q.bindValue(QStringLiteral(u":parent"), nn(project.parent));
    q.bindValue(QStringLiteral(u":created"), project.created);
    if (project.modified) q.bindValue(QStringLiteral(u":modified"), *project.modified);
    if (project.used) q.bindValue(QStringLiteral(u":used"), *project.used);
    q.bindValue(QStringLiteral(u":archived"), project.archived ? 1 : 0);
    q.bindValue(QStringLiteral(u":json"), jsonString(project.toJson()));
    if (!q.exec()) { if (error) *error = m_db.lastError(); return false; }

    QSqlQuery del(m_db.database());
    del.prepare(QStringLiteral(u"DELETE FROM project_items WHERE project_id = :id"));
    del.bindValue(QStringLiteral(u":id"), project.id);
    del.exec();

    int pos = 0;
    for (const auto& ref : project.shows) {
        QSqlQuery i(m_db.database());
        i.prepare(QStringLiteral(u"INSERT INTO project_items "
                  u"(project_id,position,show_id,layout_id,section,name,item_type,color,notes,played,json) "
                  u"VALUES (:pid,:pos,:sid,:layout,:section,:name,:type,:color,:notes,:played,:json)"));
        i.bindValue(QStringLiteral(u":pid"), project.id);
        i.bindValue(QStringLiteral(u":pos"), pos++);
        i.bindValue(QStringLiteral(u":sid"), ref.id);
        bindOrNull(i, QStringLiteral(u":layout"), ref.layout);
        i.bindValue(QStringLiteral(u":section"), ref.type ? showTypeToString(*ref.type) : QVariant{});
        bindOrNull(i, QStringLiteral(u":name"), ref.name);
        bindOrNull(i, QStringLiteral(u":color"), ref.color);
        bindOrNull(i, QStringLiteral(u":notes"), ref.notes);
        i.bindValue(QStringLiteral(u":played"), ref.played ? 1 : 0);
        i.bindValue(QStringLiteral(u":json"), jsonString(ref.toJson()));
        i.exec();
    }
    return true;
}

std::optional<Project> ShowRepository::loadProject(const Id& id) const
{
    QSqlQuery q(m_db.database());
    q.prepare(QStringLiteral(u"SELECT json FROM projects WHERE id = :id"));
    q.bindValue(QStringLiteral(u":id"), id);
    if (!q.exec() || !q.next()) return std::nullopt;
    Project p = Project::fromJson(QJsonDocument::fromJson(q.value(0).toByteArray()).object());
    p.id = id;
    return p;
}

QVector<Project> ShowRepository::listProjects() const
{
    QVector<Project> out;
    QSqlQuery q(m_db.database());
    q.exec(QStringLiteral(u"SELECT json FROM projects WHERE archived = 0 ORDER BY name COLLATE NOCASE"));
    while (q.next())
        out << Project::fromJson(QJsonDocument::fromJson(q.value(0).toByteArray()).object());
    return out;
}

bool ShowRepository::removeProject(const Id& id)
{
    QSqlQuery q(m_db.database());
    q.prepare(QStringLiteral(u"DELETE FROM projects WHERE id = :id"));
    q.bindValue(QStringLiteral(u":id"), id);
    return q.exec();
}

int ShowRepository::reindexFromDisk(QString* error)
{
    const QDir dir(QDir(m_dataDir).filePath(QStringLiteral(u"Shows")));
    const auto files = dir.entryList({QStringLiteral(u"*.show")}, QDir::Files);
    int count = 0;
    for (const auto& name : files) {
        QFile f(dir.filePath(name));
        if (!f.open(QIODevice::ReadOnly)) continue;
        const auto result = decodeShowFileRepair(f.readAll());
        f.close();
        if (!result.ok) {
            if (error) *error = name + QStringLiteral(u": ")+ result.error;
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
