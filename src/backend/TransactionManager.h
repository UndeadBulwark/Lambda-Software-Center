#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QObject>
#include <QString>

class TransactionManager : public QObject {
    Q_OBJECT
public:
    explicit TransactionManager(QObject *parent = nullptr);

signals:
    void transactionStarted(const QString &pkgId);
    void transactionProgress(const QString &pkgId, int percent, const QString &step);
    void transactionFinished(const QString &pkgId, bool success, const QString &error);
};

#endif // TRANSACTIONMANAGER_H
