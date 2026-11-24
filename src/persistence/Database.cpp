#include "Database.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace ps {

Database::Database(const QString& filePath, QObject* parent)
    : QObject(parent)
    , m_filePath(filePath)
    , m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

Database::~Database()
{
    close();
}

QString Database::defaultPath()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return QDir(base).filePath(u"proslides.db");
}

bool Database::open(QString* error)
{
    if (m_db.isValid() && m_db.isOpen()) return true;

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());

    m_db = QSqlDatabase::addDatabase(u"QSQLITE", m_connectionName);
    m_db.setDatabaseName(m_filePath);
    m_db.setConnectOptions(u"QSQLITE_BUSY_TIMEOUT=5000");

    if (!m_db.open()) {
        if (error) *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec(u"PRAGMA journal_mode=WAL");
    q.exec(u"PRAGMA synchronous=NORMAL");
    q.exec(u"PRAGMA foreign_keys=ON");
    q.exec(u"PRAGMA temp_store=MEMORY");
    q.exec(u"PRAGMA cache_size=-20000"); // ~20 MB page cache

    return applySchema(error);
}

void Database::close()
{
    if (m_db.isOpen()) m_db.close();
    if (!m_connectionName.isEmpty()) QSqlDatabase::removeDatabase(m_connectionName);
    m_db = QSqlDatabase();
}

bool Database::isOpen() const
{
    return m_db.isValid() && m_db.isOpen();
}

bool Database::transaction()
{
    return m_db.transaction();
}
bool Database::commit()
{
    return m_db.commit();
}
bool Database::rollback()
{
    return m_db.rollback();
}

QString Database::lastError() const
{
    return m_db.lastError().text();
}

namespace {

// Split a SQL script into statements, respecting string literals, line
// comments and BEGIN ... END blocks (SQLite triggers).
QStringList splitSqlStatements(const QString& sql)
{
    QStringList out;
    QString current;
    int depth = 0;
    bool inString = false;
    bool inLineComment = false;

    for (int i = 0; i < sql.size(); ++i) {
        const QChar ch = sql[i];

        if (inLineComment) {
            if (ch == u'\n') { inLineComment = false; current += ch; }
            continue;
        }
        if (inString) {
            current += ch;
            if (ch == u'\'') {
                if (i + 1 < sql.size() && sql[i + 1] == u'\'') { current += sql[++i]; }
                else inString = false;
            }
            continue;
        }
        if (ch == u'-' && i + 1 < sql.size() && sql[i + 1] == u'-') {
            inLineComment = true;
            ++i;
            continue;
        }
        if (ch == u'\'') { inString = true; current += ch; continue; }

        // word detection for BEGIN / END
        if (ch.isLetter()) {
            int j = i;
            while (j < sql.size() && sql[j].isLetter()) ++j;
            const QString word = sql.mid(i, j - i).toUpper();
            if (word == u"BEGIN") ++depth;
            else if (word == u"END" && depth > 0) --depth;
            current += sql.mid(i, j - i);
            i = j - 1;
            continue;
        }

        if (ch == u';' && depth == 0) {
            if (!current.trimmed().isEmpty()) out << current.trimmed();
            current.clear();
            continue;
        }
        current += ch;
    }
    if (!current.trimmed().isEmpty()) out << current.trimmed();
    return out;
}

} // namespace

bool Database::applySchema(QString* error)
{
    QFile file(u":/schema.sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = u"Cannot open embedded schema resource";
        return false;
    }
    const QString sql = QString::fromUtf8(file.readAll());
    file.close();

    for (const auto& statement : splitSqlStatements(sql)) {
        if (statement.startsWith(u"--")) continue;
        QSqlQuery q(m_db);
        if (!q.exec(statement)) {
            if (error) *error = q.lastError().text() + u"\nSQL: " + statement.left(160);
            return false;
        }
    }
    return true;
}

DbTransaction::DbTransaction(Database& db) : m_db(db), m_ok(db.transaction())
{
}

DbTransaction::~DbTransaction()
{
    if (m_ok) m_db.commit();
}

void DbTransaction::rollback()
{
    if (m_ok) {
        m_db.rollback();
        m_ok = false;
    }
}

} // namespace ps
