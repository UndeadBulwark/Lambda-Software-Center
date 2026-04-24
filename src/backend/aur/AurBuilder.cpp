#include "AurBuilder.h"

AurBuilder::AurBuilder(QObject *parent)
    : QObject(parent)
{
}

void AurBuilder::build(const QString &pkgName) {
    Q_UNUSED(pkgName)
}
