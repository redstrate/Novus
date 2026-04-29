// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "difftreewidget.h"
#include "hashdatabase.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KRecentFilesMenu>
#include <QApplication>
#include <QFileDialog>
#include <QListWidget>
#include <physis.hpp>

#include "hexpart.h"
#include "openinwidget.h"
#include "settings.h"

#include <QHBoxLayout>

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(data)
{
    setMinimumSize(720, 720);

    auto widget = new QWidget();
    setCentralWidget(widget);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    widget->setLayout(layout);

    m_diffTreeWidget = new DiffTreeWidget(m_database, &m_data, this);
    connect(m_diffTreeWidget, &DiffTreeWidget::bufferSelected, this, [this](physis_Buffer buffer) {
        auto decompressedBlock = physis_sqpack_read_block(Platform::Win32, buffer);
        m_hexPart->loadFile(decompressedBlock);
    });
    layout->addWidget(m_diffTreeWidget);

    m_hexPart = new HexPart(this);
    layout->addWidget(m_hexPart);

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
                openPatch(QUrl::fromLocalFile(fileName));
                m_recentFilesMenu->addUrl(QUrl::fromLocalFile(fileName));
            }
        },
        actionCollection());

    m_recentFilesMenu = new KRecentFilesMenu(this);
    actionCollection()->addAction(QStringLiteral("open_recent"), m_recentFilesMenu->menuAction());
    connect(m_recentFilesMenu, &KRecentFilesMenu::urlTriggered, this, &MainWindow::openPatch);
}

void MainWindow::openPatch(const QUrl &url)
{
    m_diffTreeWidget->openPatch(url.toLocalFile());
    setWindowTitle(url.toLocalFile());
}

#include "moc_mainwindow.cpp"
