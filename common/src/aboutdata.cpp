// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aboutdata.h"

#include <physis.hpp>

#include "novus-version.h"

#ifdef Q_OS_WIN
#include <BreezeIcons>
#include <QIcon>
#endif

void customizeAboutData(const QString &componentName, const QString &desktopFilename, const QString &applicationTitle, const QString &applicationDescription)
{
    // TODO: we shouldn't do this here
#ifdef Q_OS_WIN
    BreezeIcons::initIcons();
    QIcon::setThemeName(QStringLiteral("Breeze"));
#endif

    KAboutData about(componentName,
                     applicationTitle,
                     QStringLiteral(NOVUS_VERSION_STRING),
                     applicationDescription,
                     KAboutLicense::GPL_V3,
                     QStringLiteral("© 2022-2024 Joshua Goins"));
    about.addAuthor(QStringLiteral("Joshua Goins"),
                    QStringLiteral("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl(QStringLiteral("https://redstrate.com/rss-image.png")));
    about.setHomepage(QStringLiteral("https://xiv.zone/novus"));
    about.addComponent(QStringLiteral("physis"),
                       QStringLiteral("Library to access FFXIV data"),
                       QLatin1String(physis_get_physis_version()),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("libphysis"),
                       QStringLiteral("C bindings for physis"),
                       QLatin1String(physis_get_libphysis_version()),
                       QStringLiteral("https://git.sr.ht/~redstrate/libphysis"),
                       KAboutLicense::GPL_V3);
    about.setBugAddress(QByteArrayLiteral("https://lists.sr.ht/~redstrate/public-inbox"));
    about.setComponentName(componentName);
    about.setProgramLogo(desktopFilename);
    about.setDesktopFileName(desktopFilename);
    about.setOrganizationDomain(QByteArrayLiteral("xiv.zone"));

    KAboutData::setApplicationData(about);
}