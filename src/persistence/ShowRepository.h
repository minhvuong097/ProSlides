#pragma once

#include "core/Types.h"

#include <QVector>

namespace ps {

class Database;

struct ShowSummary {
    Id id;
    QString name;
    QString category;
    QString filePath;
    Millis created = 0;
    Millis modified = 0;
    int slideCount = 0;
};

struct SearchHit {
    Id showId;
    Id slideId;
    QString showName;
    QString group;
    QString title;
    QString snippet;
    double rank = 0.0;
};

// Persistence for shows/projects. The .show JSON file is authoritative;
// SQLite is a queryable projection rebuilt from the JSON on every save.
class ShowRepository
{
public:
    explicit ShowRepository(Database& db);
    void setDataDirectory(const QString& dir) { m_dataDir = dir; }
    QString dataDirectory() const { return m_dataDir; }

    bool saveShow(const Show& show, QString* error = nullptr);
    std::optional<Show> loadShow(const Id& id, QString* error = nullptr) const;
    bool removeShow(const Id& id);
    QVector<ShowSummary> listShows() const;

    // FTS5 search with prefix matching and diacritic-insensitive Vietnamese.
    QVector<SearchHit> search(const QString& query, int limit = 50) const;

    bool saveProject(const Project& project, QString* error = nullptr);
    std::optional<Project> loadProject(const Id& id) const;
    QVector<Project> listProjects() const;
    bool removeProject(const Id& id);

    // Reindex all .show files found under <dataDir>/Shows.
    int reindexFromDisk(QString* error = nullptr);

    static QString ftsQuery(const QString& userText);

private:
    bool indexShow(const Show& show, const QString& filePath);
    void indexSlides(const Show& show);
    void indexLayouts(const Show& show);
    void indexMedia(const Show& show);
    void indexTimeline(const QString& ownerType, const Id& ownerId,
                       const std::optional<QJsonObject>& timeline);

    Database& m_db;
    QString m_dataDir;
};

} // namespace ps
