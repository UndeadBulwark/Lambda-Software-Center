#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMetaType>

struct Package {
    enum Source { Pacman = 0, AUR, Flatpak, Unknown };
    enum InstallState { NotInstalled, Installed, UpdateAvailable };

    QString id;
    QString name;
    QString version;
    QString newVersion;
    QString description;
    QString longDescription;
    QStringList categories;
    Source source = Source::Unknown;
    InstallState state = InstallState::NotInstalled;
    qint64 installedSize = 0;
    QString downloadSize;
    QUrl iconUrl;
    QList<QUrl> screenshotUrls;
    QStringList dependencies;
    int votes = 0;           // AUR only
    float popularity = 0.0f; // AUR only
    float rating = 0.0f;     // Flatpak ODRS only
    QString flatpakRef;      // Flatpak only — full ref string e.g. "app/org.gimp.GIMP/x86_64/stable"
    QString flatpakRemote;  // Flatpak only — remote name e.g. "flathub"
    QString gitUrl;          // AUR only — https://aur.archlinux.org/<name>.git
};

Q_DECLARE_METATYPE(Package)
Q_DECLARE_METATYPE(Package::Source)
Q_DECLARE_METATYPE(Package::InstallState)
Q_DECLARE_METATYPE(QList<Package>)

#endif // PACKAGE_H
