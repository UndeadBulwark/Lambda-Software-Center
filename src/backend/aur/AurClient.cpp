#include "AurClient.h"
#include "PackageSearchUtils.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

AurClient::AurClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_debounceTimer(new QTimer(this))
{
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(250);
    connect(m_debounceTimer, &QTimer::timeout,
            this, [this]() { performSearch(m_pendingQuery); });

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, [this](QNetworkReply *reply) {
                onReplyFinished(reply);
                reply->deleteLater();
            });
}

void AurClient::search(const QString &query) {
    m_pendingQuery = query;
    m_debounceTimer->start();
}

void AurClient::info(const QStringList &pkgNames) {
    m_pendingInfoNames = pkgNames;
    performInfo(pkgNames);
}

void AurClient::performSearch(const QString &query) {
    m_pendingIsInfo = false;
#ifdef QT_TESTLIB_LIB
    if (m_useMock) {
        onMockSearch(query);
        return;
    }
#endif

    QUrl url(QString("%1/search/%2")
             .arg(m_baseUrl)
             .arg(QString::fromUtf8(QUrl::toPercentEncoding(query))));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_networkManager->get(request);
}

void AurClient::performInfo(const QStringList &pkgNames) {
    m_pendingIsInfo = true;
#ifdef QT_TESTLIB_LIB
    if (m_useMock) {
        onMockSearch(pkgNames.join(","));
        return;
    }
#endif

    QStringList args;
    for (const QString &name : pkgNames)
        args << QStringLiteral("arg[]=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(name)));

    QUrl url(QString("%1/info?%2").arg(m_baseUrl).arg(args.join("&")));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_networkManager->get(request);
}

void AurClient::onReplyFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("AUR search network error: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("AUR search: invalid JSON response");
        return;
    }

    QJsonObject root = doc.object();
    QString type = root.value("type").toString();
    if (type == "error") {
        emit errorOccurred(QString("AUR error: %1").arg(root.value("error").toString()));
        return;
    }

    QJsonArray resultsArray = root.value("results").toArray();
    QList<Package> results;
    for (const QJsonValue &v : resultsArray) {
        results.append(parsePackage(v.toObject()));
    }

    sortPackagesBySearchRelevance(results, m_pendingQuery);
    if (m_pendingIsInfo)
        emit infoFinished(results);
    else
        emit searchFinished(results);
}

#ifdef QT_TESTLIB_LIB
void AurClient::onMockSearch(const QString &query) {
    Q_UNUSED(query)
    QJsonDocument doc = QJsonDocument::fromJson(m_mockResponse.toUtf8());
    if (doc.isNull()) {
        emit errorOccurred("Injected mock JSON is invalid");
        return;
    }

    QJsonObject root = doc.object();
    QString type = root.value("type").toString();
    if (type == "error") {
        emit errorOccurred(QString("AUR error: %1").arg(root.value("error").toString()));
        return;
    }

    QJsonArray resultsArray = root.value("results").toArray();
    QList<Package> results;
    for (const QJsonValue &v : resultsArray) {
        results.append(parsePackage(v.toObject()));
    }
    sortPackagesBySearchRelevance(results, m_pendingQuery);
    if (m_pendingIsInfo)
        emit infoFinished(results);
    else
        emit searchFinished(results);
}
#endif

Package AurClient::parsePackage(const QJsonObject &obj) const {
    Package p;
    p.id = obj.value("Name").toString() + "@aur";
    p.name = obj.value("Name").toString();
    p.version = obj.value("Version").toString();
    p.description = obj.value("Description").toString().replace("\n", " ");
    p.source = Package::Source::AUR;
    p.state = Package::InstallState::NotInstalled;
    p.votes = obj.value("NumVotes").toInt();
    p.popularity = static_cast<float>(obj.value("Popularity").toDouble());
    QString urlPath = obj.value("URLPath").toString();
    if (!urlPath.isEmpty()) {
        p.iconUrl = QUrl("https://aur.archlinux.org" + urlPath);
        p.gitUrl = QStringLiteral("https://aur.archlinux.org") + urlPath;
    }
    return p;
}
