#ifndef AURBUILDER_H
#define AURBUILDER_H

#include <QObject>
#include <QString>

class AurBuilder : public QObject {
    Q_OBJECT
public:
    explicit AurBuilder(QObject *parent = nullptr);

    void build(const QString &pkgName);

signals:
    void buildProgress(const QString &pkgName, int percent, const QString &step);
    void buildFinished(const QString &pkgName, bool success, const QString &error);
};

#endif // AURBUILDER_H
