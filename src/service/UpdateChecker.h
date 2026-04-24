#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QTimer>

class UpdateChecker : public QObject {
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);

signals:
    void updatesAvailable(int count);
};

#endif // UPDATECHECKER_H
