// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "novusmainwindow.h"

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <QApplication>
#include <QDesktopServices>
#include <QMenuBar>

NovusMainWindow::NovusMainWindow()
{
    setWindowTitle(KAboutData::applicationData().displayName());
}

void NovusMainWindow::setupMenubar()
{
    auto fileMenu = menuBar()->addMenu(QStringLiteral("File"));

    setupFileMenu(fileMenu);
    if (!fileMenu->isEmpty()) {
        fileMenu->addSeparator();
    }

    auto quitAction = fileMenu->addAction(QStringLiteral("Quit"));
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("gtk-quit")));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    setupAdditionalMenus(menuBar());

    auto helpMenu = menuBar()->addMenu(QStringLiteral("Help"));

    auto donateAction = helpMenu->addAction(QStringLiteral("Donate"));
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://redstrate.com/fund")));
    });
    donateAction->setIcon(QIcon::fromTheme(QStringLiteral("help-donate")));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction(QStringLiteral("About %1").arg(KAboutData::applicationData().displayName()));
    aboutNovusAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new KAboutApplicationDialog(KAboutData::applicationData(), this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction(QStringLiteral("About Qt"));
    aboutQtAction->setIcon(QIcon(QStringLiteral(":/qt-project.org/qmessagebox/images/qtlogo-64.png")));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);
}
