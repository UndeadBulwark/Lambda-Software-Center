#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QEventLoop>
#include <QTimer>
#include <iostream>
#include "Package.h"
#include "backend/pacman/PacmanBackend.h"
#include "backend/aur/AurBackend.h"
#include "backend/flatpak/FlatpakBackend.h"
#include "models/PackageListModel.h"

static void printPackage(const Package &pkg) {
    const char *src = pkg.source == Package::Source::Pacman ? "pacman" :
                       pkg.source == Package::Source::AUR     ? "aur" : "flatpak";
    std::cout << "  [" << src << "] " << pkg.name.toStdString()
              << " " << pkg.version.toStdString() << "\n";
    if (!pkg.description.isEmpty()) {
        std::cout << "       " << pkg.description.left(100).toStdString() << "\n";
    }
}

static int runCli(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("lambda-software-center");

    QCommandLineParser parser;
    parser.setApplicationDescription("Lambda Software Center CLI");
    parser.addHelpOption();

    QCommandLineOption searchOption({"s","search"}, "Search for packages", "query");
    QCommandLineOption sourceOption({"S","source"}, "Source filter (pacman|aur|flatpak)", "source");
    QCommandLineOption listInstalledOption({"l","list-installed"}, "List installed packages");
    QCommandLineOption checkUpdatesOption({"u","check-updates"}, "Check for available updates");

    parser.addOption(searchOption);
    parser.addOption(sourceOption);
    parser.addOption(listInstalledOption);
    parser.addOption(checkUpdatesOption);
    parser.process(app);

    bool usePacman = !parser.isSet(sourceOption) || parser.value(sourceOption) == "pacman";
    bool useAur     = !parser.isSet(sourceOption) || parser.value(sourceOption) == "aur";

    if (parser.isSet(searchOption)) {
        QString query = parser.value(searchOption);
        std::cout << "Searching for: " << query.toStdString() << "\n";

        QEventLoop loop;
        int pending = 0;
        QList<Package> allResults;

        auto onResults = [&](const QList<Package> &results) {
            allResults.append(results);
            pending--;
            if (pending == 0) loop.quit();
        };
        auto onError = [](const QString &msg) {
            std::cerr << "Backend error: " << msg.toStdString() << "\n";
        };

        PacmanBackend pb;
        AurBackend ab;

        if (usePacman) {
            pending++;
            QObject::connect(&pb, &IPackageBackend::searchResultsReady, onResults);
            QObject::connect(&pb, &IPackageBackend::errorOccurred, onError);
            pb.search(query);
        }
        if (useAur) {
            pending++;
            QObject::connect(&ab, &IPackageBackend::searchResultsReady, onResults);
            QObject::connect(&ab, &IPackageBackend::errorOccurred, onError);
            ab.search(query);
        }

        if (pending == 0) {
            std::cout << "No backends selected.\n";
            return 0;
        }

        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        loop.exec();

        std::cout << "Total results: " << allResults.size() << "\n";
        for (const Package &pkg : allResults)
            printPackage(pkg);
        return 0;
    }

    if (parser.isSet(listInstalledOption)) {
        QEventLoop loop;
        PacmanBackend pb;
        QObject::connect(&pb, &IPackageBackend::installedListReady, &loop, [&loop](const QList<Package> &results) {
            std::cout << "Installed packages: " << results.size() << "\n";
            for (const Package &pkg : results)
                printPackage(pkg);
            loop.quit();
        });
        QObject::connect(&pb, &IPackageBackend::errorOccurred, &loop, [&loop](const QString &msg) {
            std::cerr << "Error: " << msg.toStdString() << "\n";
            loop.quit();
        });
        pb.listInstalled();
        return loop.exec();
    }

    if (parser.isSet(checkUpdatesOption)) {
        QEventLoop loop;
        PacmanBackend pb;
        QObject::connect(&pb, &IPackageBackend::updatesReady, &loop, [&loop](const QList<Package> &results) {
            std::cout << "Updates available: " << results.size() << "\n";
            for (const Package &pkg : results) {
                std::cout << "  " << pkg.name.toStdString() << ": "
                          << pkg.version.toStdString() << " -> "
                          << pkg.newVersion.toStdString() << "\n";
            }
            loop.quit();
        });
        QObject::connect(&pb, &IPackageBackend::errorOccurred, &loop, [&loop](const QString &msg) {
            std::cerr << "Error: " << msg.toStdString() << "\n";
            loop.quit();
        });
        pb.checkUpdates();
        return loop.exec();
    }

    parser.showHelp();
    return 0;
}

static int runGui(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("lambda-software-center");

    qRegisterMetaType<Package>("Package");
    qRegisterMetaType<Package::Source>("Package::Source");
    qRegisterMetaType<Package::InstallState>("Package::InstallState");
    qRegisterMetaType<QList<Package>>("QList<Package>");

    PacmanBackend *pacmanBackend = new PacmanBackend();
    AurBackend *aurBackend = new AurBackend();
    FlatpakBackend *flatpakBackend = new FlatpakBackend();

    PackageListModel *searchModel = new PackageListModel();

    QObject::connect(pacmanBackend, &IPackageBackend::searchResultsReady,
                     searchModel, [searchModel](const QList<Package> &results) {
                         QList<Package> merged = searchModel->packages();
                         merged.append(results);
                         searchModel->setPackages(merged);
                     });

    QObject::connect(aurBackend, &IPackageBackend::searchResultsReady,
                     searchModel, [searchModel](const QList<Package> &results) {
                         QList<Package> merged = searchModel->packages();
                         merged.append(results);
                         searchModel->setPackages(merged);
                     });

    QObject::connect(flatpakBackend, &IPackageBackend::searchResultsReady,
                     searchModel, [searchModel](const QList<Package> &results) {
                         QList<Package> merged = searchModel->packages();
                         merged.append(results);
                         searchModel->setPackages(merged);
                     });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("searchModel", searchModel);
    engine.rootContext()->setContextProperty("pacmanBackend", pacmanBackend);
    engine.rootContext()->setContextProperty("aurBackend", aurBackend);
    engine.rootContext()->setContextProperty("flatpakBackend", flatpakBackend);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

int main(int argc, char *argv[])
{
    bool isCli = false;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg.startsWith("-s") || arg.startsWith("--search")
            || arg.startsWith("-l") || arg.startsWith("--list-installed")
            || arg.startsWith("-u") || arg.startsWith("--check-updates")) {
            isCli = true;
            break;
        }
    }

    if (isCli)
        return runCli(argc, argv);
    else
        return runGui(argc, argv);
}
