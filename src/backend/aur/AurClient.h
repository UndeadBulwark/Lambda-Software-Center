#ifndef AURCLIENT_H
#define AURCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QList>
#include <QTimer>
#include "Package.h"

class AurClient : public QObject {
    Q_OBJECT
public:
    explicit AurClient(QObject *parent = nullptr);

    void search(const QString &query);
    void info(const QStringList &pkgNames);

signals:
    void searchFinished(QList<Package> results);
    void infoFinished(QList<Package> results);
    void errorOccurred(const QString &message);

#ifdef QT_TESTLIB_LIB
public:
    inline void setBaseUrl(const QString &url) { m_baseUrl = url; }
    inline void injectMockResponse(const QString &json) {
        m_mockResponse = json;
        m_useMock = true;
    }

private:
    QString m_mockResponse;
    bool m_useMock = false;
#endif

private:
    QNetworkAccessManager *m_networkManager;
    QTimer *m_debounceTimer;

    QString m_pendingQuery;
    QStringList m_pendingInfoNames;
    bool m_pendingIsInfo = false;
    QString m_baseUrl = QStringLiteral("https://aur.archlinux.org/rpc/v5");

    void performSearch(const QString &query);
    void performInfo(const QStringList &pkgNames);
    void onReplyFinished(QNetworkReply *reply);
    void onMockSearch(const QString &query);
    Package parsePackage(const QJsonObject &obj) const;
};

#endif // AURCLIENT_H
