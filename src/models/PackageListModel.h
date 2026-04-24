#ifndef PACKAGELISTMODEL_H
#define PACKAGELISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "Package.h"

class PackageListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit PackageListModel(QObject *parent = nullptr);

    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        VersionRole,
        NewVersionRole,
        DescriptionRole,
        LongDescriptionRole,
        CategoriesRole,
        SourceRole,
        StateRole,
        InstalledSizeRole,
        DownloadSizeRole,
        IconUrlRole,
        ScreenshotUrlsRole,
        DependenciesRole,
        VotesRole,
        PopularityRole,
        RatingRole,
        FlatpakRefRole
    };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setPackages(const QList<Package> &packages);
    QList<Package> packages() const;
    Q_INVOKABLE void clear();
    Q_INVOKABLE void appendPackages(const QList<Package> &packages);

private:
    QList<Package> m_packages;
};

#endif // PACKAGELISTMODEL_H
