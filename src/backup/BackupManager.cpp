#include "BackupManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUuid>

namespace ps::backup {

BackupManager::BackupManager(QObject* parent) : QObject(parent)
{
}

QString BackupManager::backupRoot() const
{
    return QDir(m_dataDir).filePath(QStringLiteral(u"Backups"));
}

bool BackupManager::copyTree(const QString& source, const QString& destination)
{
    const QDir src(source);
    if (!src.exists()) return true;
    QDir().mkpath(destination);
    for (const auto& entry : src.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString from = src.filePath(entry);
        const QString to = QDir(destination).filePath(entry);
        const QFileInfo info(from);
        if (info.isDir()) {
            if (!copyTree(from, to)) return false;
        } else {
            if (QFile::exists(to)) QFile::remove(to);
            if (!QFile::copy(from, to)) return false;
        }
    }
    return true;
}

QString BackupManager::backupNow(const QString& label)
{
    if (m_dataDir.isEmpty()) {
        emit errorOccurred(QStringLiteral(u"Data directory not configured"));
        return {};
    }
    const QString id = QDateTime::currentDateTime().toString(QStringLiteral(u"yyyyMMdd-HHmmss")) +
                       QStringLiteral(u"-")+ QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);
    const QString target = QDir(backupRoot()).filePath(id);
    QDir().mkpath(QDir(target).filePath(QStringLiteral(u"Shows")));
    QDir().mkpath(QDir(target).filePath(QStringLiteral(u"Config")));

    if (!copyTree(QDir(m_dataDir).filePath(QStringLiteral(u"Shows")), QDir(target).filePath(QStringLiteral(u"Shows"))) ||
        !copyTree(QDir(m_dataDir).filePath(QStringLiteral(u"Config")), QDir(target).filePath(QStringLiteral(u"Config")))) {
        emit errorOccurred(QStringLiteral(u"Failed to copy show files"));
        return {};
    }
    QFile::copy(QDir(m_dataDir).filePath(QStringLiteral(u"proslides.db")), QDir(target).filePath(QStringLiteral(u"proslides.db")));

    QJsonObject manifest;
    manifest.insert(QStringLiteral(u"id"), id);
    manifest.insert(QStringLiteral(u"label"), label.isEmpty() ? QStringLiteral(u"Automatic backup"): label);
    manifest.insert(QStringLiteral(u"created"), QDateTime::currentMSecsSinceEpoch());
    manifest.insert(QStringLiteral(u"version"), QStringLiteral(u"0.10"));
    QFile file(QDir(target).filePath(QStringLiteral(u"manifest.json")));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(manifest).toJson(QJsonDocument::Indented));
        file.close();
    }
    refresh();
    return id;
}

bool BackupManager::restore(const QString& backupId)
{
    const QString source = QDir(backupRoot()).filePath(backupId);
    if (!QDir(source).exists()) {
        emit errorOccurred(QStringLiteral(u"Backup not found"));
        return false;
    }
    // Restore shows only; database is rebuilt by ShowRepository::reindexFromDisk.
    if (!copyTree(QDir(source).filePath(QStringLiteral(u"Shows")), QDir(m_dataDir).filePath(QStringLiteral(u"Shows")))) {
        emit errorOccurred(QStringLiteral(u"Failed to restore shows"));
        return false;
    }
    return true;
}

bool BackupManager::remove(const QString& backupId)
{
    const QString path = QDir(backupRoot()).filePath(backupId);
    if (!QDir(path).exists()) return false;
    const bool ok = QDir(path).removeRecursively();
    refresh();
    return ok;
}

void BackupManager::refresh()
{
    m_model.clear();
    const QDir root(backupRoot());
    if (root.exists()) {
        for (const auto& id : root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            QFile file(root.filePath(id + QStringLiteral(u"/manifest.json")));
            QJsonObject manifest;
            if (file.open(QIODevice::ReadOnly)) {
                manifest = QJsonDocument::fromJson(file.readAll()).object();
                file.close();
            }
            QVariantMap m;
            m.insert(QStringLiteral(u"id"), id);
            m.insert(QStringLiteral(u"label"), manifest.value(QStringLiteral(u"label")).toString());
            m.insert(QStringLiteral(u"created"), manifest.value(QStringLiteral(u"created")).toVariant());
            m_model.append(m);
        }
    }
    emit backupsChanged();
}

int BackupManager::prune(int keep)
{
    refresh();
    int removed = 0;
    while (m_model.size() > keep && !m_model.isEmpty()) {
        if (remove(m_model.first().toMap().value(QStringLiteral(u"id")).toString())) ++removed;
        else break;
    }
    return removed;
}

} // namespace ps::backup
