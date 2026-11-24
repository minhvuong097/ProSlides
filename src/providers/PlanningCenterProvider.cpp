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
        emit errorOccurred(u"Planning Center credentials are not configured");
        return;
    }
    QUrl url(u"https://api.planningcenteronline.com/services/v2/plans");
    QUrlQuery query;
    query.addQueryItem(u"per_page", u"50");
    if (!fromDate.isEmpty()) query.addQueryItem(u"filter[from]", fromDate);
    if (!toDate.isEmpty()) query.addQueryItem(u"filter[to]", toDate);
    query.addQueryItem(u"include", u"items");
    url.setQuery(query);

    QNetworkRequest request(url);
    const QByteArray credentials = (m_appId + u":" + m_secret).toUtf8();
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

    const auto data = m_lastResponse.object().value(u"data").toArray();
    const auto included = m_lastResponse.object().value(u"included").toArray();

    QMap<QString, QString> itemTitles;
    for (const auto& item : included) {
        if (!item.isObject()) continue;
        const auto obj = item.toObject();
        if (obj.value(u"type").toString() != u"Item") continue;
        itemTitles.insert(obj.value(u"id").toString(),
                          obj.value(u"attributes").toObject().value(u"title").toString());
    }

    for (const auto& value : data) {
        if (!value.isObject()) continue;
        const auto obj = value.toObject();
        const auto attrs = obj.value(u"attributes").toObject();
        QVariantMap plan;
        plan.insert(u"id", obj.value(u"id").toString());
        plan.insert(u"title", attrs.value(u"title").toString());
        plan.insert(u"date", attrs.value(u"date").toString());
        const auto items = obj.value(u"relationships").toObject()
                              .value(u"items").toObject().value(u"data").toArray();
        QStringList titles;
        for (const auto& ref : items) {
            if (!ref.isObject()) continue;
            const QString id = ref.toObject().value(u"id").toString();
            if (itemTitles.contains(id)) titles << itemTitles.value(id);
        }
        plan.insert(u"items", titles);
        m_plans.append(plan);
    }
    emit plansChanged();
}

bool PlanningCenterProvider::importPlan(const QString& planId)
{
    if (!m_repository) {
        emit errorOccurred(u"Repository not available");
        return false;
    }

    QVariantMap selected;
    for (const auto& value : m_plans) {
        if (value.toMap().value(u"id").toString() == planId) {
            selected = value.toMap();
            break;
        }
    }
    if (selected.isEmpty()) return false;

    Project project;
    project.id = makeId();
    project.name = selected.value(u"title").toString();
    project.notes = u"Imported from Planning Center (" + selected.value(u"date").toString() + u")";
    project.created = nowMillis();
    project.parent = u"root";

    const auto titles = selected.value(u"items").toStringList();
    for (const auto& title : titles) {
        ProjectShowRef ref;
        ref.id = makeId();
        ref.name = title;
        ref.type = ShowType::Show;
        project.shows.push_back(std::move(ref));
    }
    if (!m_repository->saveProject(project)) {
        emit errorOccurred(u"Failed to save imported project");
        return false;
    }
    emit planImported(project.id);
    return true;
}

} // namespace ps::providers
