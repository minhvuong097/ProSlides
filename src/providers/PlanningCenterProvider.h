#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

namespace ps {
class ShowRepository;
}

namespace ps::providers {

// Minimal Planning Center Services client. Uses the official REST API with
// app id / secret basic auth and converts plans into ProSlides projects.
class PlanningCenterProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList plans READ plans NOTIFY plansChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY configuredChanged)

public:
    explicit PlanningCenterProvider(ShowRepository* repository, QObject* parent = nullptr);

    void setRepository(ShowRepository* repository) { m_repository = repository; }
    void setCredentials(const QString& appId, const QString& secret);

    QVariantList plans() const { return m_plans; }
    bool configured() const { return !m_appId.isEmpty() && !m_secret.isEmpty(); }

    Q_INVOKABLE void fetch(const QString& fromDate = {}, const QString& toDate = {});
    Q_INVOKABLE bool importPlan(const QString& planId);

signals:
    void plansChanged();
    void configuredChanged();
    void planImported(const QString& projectId);
    void errorOccurred(const QString& error);

private slots:
    void onFinished(QNetworkReply* reply);

private:
    ShowRepository* m_repository = nullptr;
    QNetworkAccessManager m_network;
    QString m_appId;
    QString m_secret;
    QVariantList m_plans;
    QJsonDocument m_lastResponse;
};

} // namespace ps::providers
