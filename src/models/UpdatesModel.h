#ifndef UPDATESMODEL_H
#define UPDATESMODEL_H

#include "PackageListModel.h"

class UpdatesModel : public PackageListModel {
    Q_OBJECT
public:
    explicit UpdatesModel(QObject *parent = nullptr);
};

#endif // UPDATESMODEL_H
