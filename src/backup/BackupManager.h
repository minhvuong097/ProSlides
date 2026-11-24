#pragma once

#include <QObject>
#include <QVariantList>
#include <QDateTime>

namespace ps::backup {

// Creates timestamped snapshots of the data directory (shows, config, DB)
// and can restore them. Compatible with FreeShow's Backups/ layout.
class BackupManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList backups READ backups NOTIFY backupsChanged)

public:
    explicit BackupManager(QObject* parent = nullptr);

    void setDataDirectory(const QString& dir) { m_dataDir = dir; }
    QString dataDirectory() const { return m_dataDir; }

    QVariantList backups() const { return m_model; }

    Q_INVOKABLE QString backupNow(const QString& label = {});
    Q_INVOKABLE bool restore(const QString& backupId);
    Q_INVOKABLE bool remove(const QString& backupId);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE int prune(int keep = 10);

signals:
    void backupsChanged();
    void errorOccurred(const QString& error);

private:
    bool copyTree(const QString& source, const QString& destination);
    QString backupRoot() const;

    QString m_dataDir;
    QVariantList m_model;
};

} // namespace ps::backup
