#include "PlanningCenterProvider.h"
#include "persistence/ShowRepository.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QVariantMap>
#include <QMap>

namespace ps::providers {

PlanningCenterProvider::PlanningCenterProvider(ShowRepository* repository, QObject* parent)
    : QObject(parent), m_repository(repository)
{
    connect(&m_network, &QNetworkAccessManager::finished,
            this, &PlanningCenterProvider::onFinished);
}

void PlanningCenterProvider::setCredentials(const QString& appId, const QString& secret)
{
    m_appId = appId;
    m_secret = secret;
    emit configuredChanged();
}

void PlanningCenterProvider::fetch(const QString& fromDate, const QString& toDate)
{
    if (!configured()) {
        emit errorOccurred(QStringLiteral(u"Planning Center credentials are not configured"));
        return;
    }
    QUrl url(QStringLiteral(u"https://api.planningcenteronline.com/services/v2/plans"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral(u"per_page"), QStringLiteral(u"50"));
    if (!fromDate.isEmpty()) query.addQueryItem(QStringLiteral(u"filter[from]"), fromDate);
    if (!toDate.isEmpty()) query.addQueryItem(QStringLiteral(u"filter[to]"), toDate);
    query.addQueryItem(QStringLiteral(u"include"), QStringLiteral(u"items"));
    url.setQuery(query);

    QNetworkRequest request(url);
    const QByteArray credentials = (m_appId + QStringLiteral(u":")+ m_secret).toUtf8();
    request.setRawHeader("Authorization", "Basic " + credentials.toBase64());
    request.setRawHeader("Accept", "application/json");
    m_network.get(request);
}

void PlanningCenterProvider::onFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }
    m_lastResponse = QJsonDocument::fromJson(reply->readAll());
    m_plans.clear();

    const auto data = m_lastResponse.object().value(QStringLiteral(u"data")).toArray();
    const auto included = m_lastResponse.object().value(QStringLiteral(u"included")).toArray();

    QMap<QString, QString> itemTitles;
    for (const auto& item : included) {
        if (!item.isObject()) continue;
        const auto obj = item.toObject();
        if (obj.value(QStringLiteral(u"type")).toString() != QStringLiteral(u"Item")) continue;
        itemTitles.insert(obj.value(QStringLiteral(u"id")).toString(),
                          obj.value(QStringLiteral(u"attributes")).toObject().value(QStringLiteral(u"title")).toString());
    }

    for (const auto& value : data) {
        if (!value.isObject()) continue;
        const auto obj = value.toObject();
        const auto attrs = obj.value(QStringLiteral(u"attributes")).toObject();
        QVariantMap plan;
        plan.insert(QStringLiteral(u"id"), obj.value(QStringLiteral(u"id")).toString());
        plan.insert(QStringLiteral(u"title"), attrs.value(QStringLiteral(u"title")).toString());
        plan.insert(QStringLiteral(u"date"), attrs.value(QStringLiteral(u"date")).toString());
        const auto items = obj.value(QStringLiteral(u"relationships")).toObject()
                              .value(QStringLiteral(u"items")).toObject().value(QStringLiteral(u"data")).toArray();
        QStringList titles;
        for (const auto& ref : items) {
            if (!ref.isObject()) continue;
            const QString id = ref.toObject().value(QStringLiteral(u"id")).toString();
            if (itemTitles.contains(id)) titles << itemTitles.value(id);
        }
        plan.insert(QStringLiteral(u"items"), titles);
        m_plans.append(plan);
    }
    emit plansChanged();
}

bool PlanningCenterProvider::importPlan(const QString& planId)
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral(u"Repository not available"));
        return false;
    }

    QVariantMap selected;
    for (const auto& value : m_plans) {
        if (value.toMap().value(QStringLiteral(u"id")).toString() == planId) {
            selected = value.toMap();
            break;
        }
    }
    if (selected.isEmpty()) return false;

    Project project;
    project.id = makeId();
    project.name = selected.value(QStringLiteral(u"title")).toString();
    project.notes = QStringLiteral(u"Imported from Planning Center (")+ selected.value(QStringLiteral(u"date")).toString() + QStringLiteral(u")");
    project.created = nowMillis();
    project.parent = QStringLiteral(u"root");

    const auto titles = selected.value(QStringLiteral(u"items")).toStringList();
    for (const auto& title : titles) {
        ProjectShowRef ref;
        ref.id = makeId();
        ref.name = title;
        ref.type = ShowType::Show;
        project.shows.push_back(std::move(ref));
    }
    if (!m_repository->saveProject(project)) {
        emit errorOccurred(QStringLiteral(u"Failed to save imported project"));
        return false;
    }
    emit planImported(project.id);
    return true;
}

} // namespace ps::providers
