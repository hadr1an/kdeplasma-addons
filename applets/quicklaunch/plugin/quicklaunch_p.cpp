/*
 *  Copyright 2008-2009 Lukas Appelhans <l.appelhans@gmx.de>
 *  Copyright 2010-2011 Ingomar Wesp <ingomar@wesp.name>
 *  Copyright 2013 Bhushan Shah <bhush94@gmail.com>
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "quicklaunch_p.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QStandardPaths>

#include <KRun>
#include <KConfig>
#include <KConfigGroup>
#include <KFileItem>
#include <KDesktopFile>
#include <KOpenWithDialog>
#include <KPropertiesDialog>

#include <kio/global.h>

QuicklaunchPrivate::QuicklaunchPrivate(QObject *parent)
    : QObject(parent)
{
}

QJsonObject QuicklaunchPrivate::launcherData(const QUrl &url)
{
    QString name;
    QString icon;
    QString genericName;

    if (url.scheme() == QLatin1String("quicklaunch")) {
        // Ignore internal scheme
    } else if (url.isLocalFile()) {
        const KFileItem fileItem(url);
        const QFileInfo fi(url.toLocalFile());

        if (fileItem.isDesktopFile()) {
            const KDesktopFile f(url.toLocalFile());
            name = f.readName();
            icon = f.readIcon();
            genericName = f.readGenericName();
            if (name.isEmpty()) {
                name = QFileInfo(url.toLocalFile()).fileName();
            }
        } else {
            QMimeDatabase db;
            name = fi.baseName();
            icon = db.mimeTypeForUrl(url).iconName();
            genericName = fi.baseName();
        }
    } else {
        if (url.scheme().contains(QLatin1String("http"))) {
            name = url.host();
        } else if (name.isEmpty()) {
            name = url.toString();
            if (name.endsWith(QLatin1String(":/"))) {
                name = url.scheme();
            }
        }
        icon = KIO::iconNameForUrl(url);
    }

    QJsonObject data;
    data[QStringLiteral("applicationName")] = name;
    data[QStringLiteral("iconName")] = icon;
    data[QStringLiteral("genericName")] = genericName;
    return data;
}

void QuicklaunchPrivate::openUrl(const QUrl &url)
{
    new KRun(url, Q_NULLPTR);
}

QImage QuicklaunchPrivate::iconFromTheme(const QString &iconName, const QSize &size)
{
    return QIcon::fromTheme(iconName).pixmap(size).toImage();
}

void QuicklaunchPrivate::addLauncher(bool isPopup)
{
    KOpenWithDialog *dialog = new KOpenWithDialog();
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->hideRunInTerminal();
    dialog->setSaveNewApplications(true);
    dialog->show();

    connect(dialog, &KOpenWithDialog::accepted, this, [this, dialog, isPopup]() {
        const QUrl &url = QUrl::fromLocalFile(dialog->service()->entryPath());
        if (url.isValid()) {
            Q_EMIT launcherAdded(url.toString(), isPopup);
        }
    });
}

static QString locateLocal(const QString &file)
{
    const QString &dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    const QString appDataPath = QStringLiteral("%1/quicklaunch").arg(dataPath);
    QDir().mkpath(appDataPath);
    return QStringLiteral("%1/%2").arg(appDataPath, file);
}

static QString determineNewDesktopFilePath(const QString &baseName)
{
    QString appendix;
    QString desktopFilePath = locateLocal(baseName) + QLatin1String(".desktop");

    while (QFile::exists(desktopFilePath)) {
        if (appendix.isEmpty()) {
            qsrand(QDateTime::currentDateTime().toTime_t());
            appendix += QLatin1Char('-');
        }

        // Limit to [0-9] and [a-z] range.
        char newChar = qrand() % 36;
        newChar += newChar < 10 ? 48 : 97-10;
        appendix += newChar;

        desktopFilePath = locateLocal(baseName + appendix + QLatin1String(".desktop"));
    }

    return desktopFilePath;
}

void QuicklaunchPrivate::editLauncher(QUrl url, int index, bool isPopup)
{
    // If the launcher does not point to a desktop file, create one,
    // so that user can change url, icon, text and description.
    bool desktopFileCreated = false;

    if (!url.isLocalFile() || !KDesktopFile::isDesktopFile(url.toLocalFile())) {
        QString desktopFilePath = determineNewDesktopFilePath(QStringLiteral("launcher"));
        QJsonObject data = launcherData(url);

        KConfig desktopFile(desktopFilePath);
        KConfigGroup desktopEntry(&desktopFile, "Desktop Entry");

        desktopEntry.writeEntry("Name", data.value(QStringLiteral("applicationName")).toString());
        desktopEntry.writeEntry("Comment", data.value(QStringLiteral("genericName")).toString());
        desktopEntry.writeEntry("Icon", data.value(QStringLiteral("iconName")).toString());
        desktopEntry.writeEntry("Type", "Link");
        desktopEntry.writeEntry("URL", url);

        desktopEntry.sync();

        url = QUrl::fromLocalFile(desktopFilePath);
        desktopFileCreated = true;
    }

    KPropertiesDialog *dialog = new KPropertiesDialog(url);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

    connect(dialog, &KPropertiesDialog::accepted, this, [this, dialog, index, isPopup]() {
        QUrl url = dialog->url();
        QString path = url.toLocalFile();

        // If the user has renamed the file, make sure that the new
        // file name has the extension ".desktop".
        if (!path.endsWith(QLatin1String(".desktop"))) {
            QFile::rename(path, path + QLatin1String(".desktop"));
            path += QLatin1String(".desktop");
            url = QUrl::fromLocalFile(path);
        }
        Q_EMIT launcherEdited(url.toString(), index, isPopup);
    });

    connect(dialog, &KPropertiesDialog::rejected, this, [this, url, desktopFileCreated]() {
        if (desktopFileCreated) {
            // User didn't save the data, delete the temporary desktop file.
            QFile::remove(url.toLocalFile());
        }
    });
}