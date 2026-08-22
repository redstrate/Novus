// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "cutbpart.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KLocalizedString>
#include <QApplication>
#include <QDesktopServices>
#include <glm/gtc/type_ptr.hpp>
#include <physis.hpp>

#include "cutscenelistwidget.h"
#include "openinwidget.h"

#include <KConfigGroup>
#include <QDirIterator>

MainWindow::MainWindow(const physis_SqPackResource data)
    : m_cache(data)
{
    setupActions();
    setupGUI(QSize(1280, 720), Default, QStringLiteral("cutsceneeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
    // We don't use this well enough
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::WhatsThis)));

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);

    m_part = new CutbPart(m_cache, this);
    setCentralWidget(m_part);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupActions()
{
    KStandardAction::open(
        qApp,
        [this] {
            auto listWidget = new CutsceneListWidget(m_cache, this);
            connect(listWidget, &CutsceneListWidget::accepted, this, [this, listWidget] {
                const auto cutscene = QStringLiteral("cut/%1.cutb").arg(listWidget->acceptedCutscene());
                const auto cutsceneFile = m_cache.read(cutscene);
                if (cutsceneFile.data) {
                    m_part->load(m_cache.platform(), cutsceneFile);
                } else {
                    qWarning() << "Unable to load cutscene file" << cutscene;
                }
            });
            listWidget->show();
        },
        actionCollection());

    // Window color scheme menu
    const auto manager = KColorSchemeManager::instance();
    const auto selectionMenu = KColorSchemeMenu::createMenu(manager, this);
    const auto windowColorSchemeMenu = new QAction(this);
    windowColorSchemeMenu->setMenu(selectionMenu->menu());
    windowColorSchemeMenu->menu()->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
    windowColorSchemeMenu->menu()->setTitle(i18n("&Window Color Scheme"));
    actionCollection()->addAction(QStringLiteral("window_color_scheme"), windowColorSchemeMenu);
}

#include "moc_mainwindow.cpp"
