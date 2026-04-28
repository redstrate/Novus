// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "difftreewidget.h"
#include "hashdatabase.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QFileDialog>
#include <QListWidget>
#include <physis.hpp>

#include "openinwidget.h"
#include "settings.h"

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(data)
{
    setMinimumSize(720, 720);

    m_diffTreeWidget = new DiffTreeWidget(m_database, &m_data, this);
    setCentralWidget(m_diffTreeWidget);

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("patchdiff.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
}

MainWindow::~MainWindow()
{
    physis_sqpack_free(&m_data);
}

void MainWindow::setupActions()
{
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
    KStandardAction::open(
        qApp,
        [this] {
            const auto fileName = QFileDialog::getOpenFileName(this, i18n("Open Patch"), QDir::homePath(), i18n("ZiPatch files (*.patch)"));
            if (!fileName.isEmpty()) {
                m_diffTreeWidget->openPatch(fileName);
                setWindowTitle(fileName);
            }
        },
        actionCollection());
}

#include "moc_mainwindow.cpp"
