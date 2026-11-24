#pragma once

#include <QObject>
#include <QSqlDatabase>

namespace ps {

// Owns the application's SQLite connection. Use one instance per thread.
class Database : public QObject
{
public:
    Database();
    explicit Database(const QString& filePath, QObject* parent = nullptr);
    ~Database() override;

    bool open(QString* error = nullptr);
    void close();

    bool isOpen() const;
    QSqlDatabase database() const { return m_db; }
    QString filePath() const { return m_filePath; }

    bool transaction();
    bool commit();
    bool rollback();

    QString lastError() const;

    static QString defaultPath(); // %APPDATA%/ProSlides/proslides.db

private:
    bool applySchema(QString* error = nullptr);

    QString m_filePath;
    QString m_connectionName;
    QSqlDatabase m_db;
};

class DbTransaction
{
public:
    explicit DbTransaction(Database& db);
    ~DbTransaction();

    bool ok() const { return m_ok; }
    void rollback();

private:
    Database& m_db;
    bool m_ok;
};

} // namespace ps
