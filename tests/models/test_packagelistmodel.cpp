#include <QtTest>
#include "PackageListModel.h"

class TestPackageListModel : public QObject
{
    Q_OBJECT

private slots:

    void testAppendPackagesAccumulates()
    {
        PackageListModel model;

        Package a;
        a.name = "pkg-a";

        Package b;
        b.name = "pkg-b";

        model.setPackages({a});
        QCOMPARE(model.rowCount(), 1);

        model.appendPackages({b});
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.data(model.index(1), PackageListModel::NameRole).toString(),
                 QString("pkg-b"));
    }

    void testAppendPackagesEmptyIsNoOp()
    {
        PackageListModel model;

        Package a;
        a.name = "pkg-a";

        model.setPackages({a});
        QCOMPARE(model.rowCount(), 1);

        QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);
        model.appendPackages({});
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(spy.count(), 0);
    }

    void testAppendMultipleAtOnce()
    {
        PackageListModel model;

        Package a; a.name = "a";
        Package b; b.name = "b";
        Package c; c.name = "c";

        QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);
        model.appendPackages({a, b, c});

        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(spy.count(), 1);

        QList<QVariant> args = spy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 0);  // first row
        QCOMPARE(args.at(2).toInt(), 2);  // last row
    }

    void testSetPackagesReplaces()
    {
        PackageListModel model;

        Package a; a.name = "old";
        model.appendPackages({a});
        QCOMPARE(model.rowCount(), 1);

        Package b; b.name = "new";
        model.setPackages({b});
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0), PackageListModel::NameRole).toString(),
                 QString("new"));
    }

    void testClearEmptiesModel()
    {
        PackageListModel model;

        Package a; a.name = "a";
        model.appendPackages({a});
        QCOMPARE(model.rowCount(), 1);

        model.clear();
        QCOMPARE(model.rowCount(), 0);
    }

    void testPackagesReturnsInternalList()
    {
        PackageListModel model;

        Package a; a.name = "a";
        Package b; b.name = "b";

        model.setPackages({a, b});
        QList<Package> returned = model.packages();
        QCOMPARE(returned.size(), 2);
        QCOMPARE(returned.at(0).name, QString("a"));
        QCOMPARE(returned.at(1).name, QString("b"));
    }
};

QTEST_MAIN(TestPackageListModel)
#include "test_packagelistmodel.moc"
