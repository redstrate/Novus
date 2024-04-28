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
    about.addCredit(QStringLiteral("RenderDoc"),
                    QStringLiteral("A fantastic and always reliable tool for debugging problems of my own creation"),
                    {},
                    QStringLiteral("https://renderdoc.org/"));
    about.addCredit(QStringLiteral("Dalamud VFX Editor"),
                    QStringLiteral("Providing an easy way to poke at the game's graphics"),
                    {},
                    QStringLiteral("https://github.com/0ceal0t/Dalamud-VFXEditor"));
    about.addCredit(QStringLiteral("Ouroboros"),
                    QStringLiteral("Reverse engineering the game's shaders"),
                    {},
                    QStringLiteral("https://github.com/Shaderlayan/Ouroboros"));
    about.addCredit(QStringLiteral("crcracker"),
                    QStringLiteral("Useful tool for figuring out shader keys"),
                    {},
                    QStringLiteral("https://github.com/NotNite/crcracker"));
    about.setHomepage(QStringLiteral("https://xiv.zone/novus"));
    about.addComponent(QStringLiteral("Physis"),
                       QStringLiteral("Library for reading and writing FFXIV data"),
                       QStringLiteral("%1 (libphysis: %2)").arg(QLatin1String(physis_get_physis_version()), QLatin1String(physis_get_libphysis_version())),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("Vulkan"),
                       QStringLiteral("Cross-platform 3D graphics and computing programming interface"),
                       {},
                       QStringLiteral("https://www.vulkan.org/"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("SPIRV-Cross"),
                       QStringLiteral("Library for performing reflection on SPIR-V and disassembling SPIR-V back to high level languages"),
                       {},
                       QStringLiteral("https://github.com/KhronosGroup/SPIRV-Cross"),
                       KAboutLicense::Unknown);
    about.addComponent(QStringLiteral("glm"),
                       QStringLiteral("C++ mathematics library for graphics software"),
                       {},
                       QStringLiteral("https://glm.g-truc.net/"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("tinygltf"),
                       QStringLiteral("Header only C++11 tiny glTF 2.0 library"),
                       {},
                       QStringLiteral("https://github.com/syoyo/tinygltf"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("DXVK"),
                       QStringLiteral("Conversion from DXBC to SPIRV bytecode"),
                       {},
                       QStringLiteral("https://github.com/doitsujin/dxvk"),
                       KAboutLicense::Unknown);
    about.addComponent(QStringLiteral("Dear ImGui"),
                       QStringLiteral("Bloat-free Graphical User interface for C++ with minimal dependencies"),
                       {},
                       QStringLiteral("https://github.com/ocornut/imgui"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("magic_enum"),
                       QStringLiteral("Static reflection for enums for modern C++"),
                       {},
                       QStringLiteral("https://github.com/Neargye/magic_enum"),
                       KAboutLicense::MIT);
    about.setBugAddress(QByteArrayLiteral("https://lists.sr.ht/~redstrate/public-inbox"));
    about.setComponentName(componentName);
    about.setProgramLogo(desktopFilename);
    about.setDesktopFileName(desktopFilename);
    about.setOrganizationDomain(QByteArrayLiteral("xiv.zone"));

    KAboutData::setApplicationData(about);
}