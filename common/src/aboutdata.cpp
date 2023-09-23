#include "aboutdata.h"

#include <physis.hpp>

#include "novus-version.h"

void customizeAboutData(
    const QString& componentName,
    const QString& applicationTitle,
    const QString& applicationDescription) {
    KAboutData about(
        componentName,
        applicationTitle,
        QStringLiteral(NOVUS_VERSION_STRING),
        applicationDescription,
        KAboutLicense::GPL_V3,
        QStringLiteral("© 2023 Joshua Goins"));
    about.addAuthor(
        QStringLiteral("Joshua Goins"),
        QStringLiteral("Maintainer"),
        QStringLiteral("josh@redstrate.com"),
        QStringLiteral("https://redstrate.com/"));
    about.setHomepage(QStringLiteral("https://xiv.zone/astra"));
    about.addComponent(
        QStringLiteral("physis"),
        QStringLiteral("Library to access FFXIV data"),
        physis_get_physis_version(),
        QStringLiteral("https://xiv.zone/physis"),
        KAboutLicense::GPL_V3);
    about.addComponent(
        QStringLiteral("libphysis"),
        QStringLiteral("C bindings for physis"),
        physis_get_libphysis_version(),
        {},
        KAboutLicense::GPL_V3);
    about.setBugAddress(QByteArrayLiteral("https://lists.sr.ht/~redstrate/public-inbox"));
    about.setComponentName(componentName);

    KAboutData::setApplicationData(about);
}