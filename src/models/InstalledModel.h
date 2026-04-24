#ifndef INSTALLEDMODEL_H
#define INSTALLEDMODEL_H

#include "PackageListModel.h"

class InstalledModel : public PackageListModel {
    Q_OBJECT
public:
    explicit InstalledModel(QObject *parent = nullptr);
};

#endif // INSTALLEDMODEL_H
