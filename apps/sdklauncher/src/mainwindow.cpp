// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFormLayout>
#include <QListWidget>
#include <QProcess>
#include <QUrl>
#include <QVBoxLayout>

#include "launcherconfig.h"
#include "settings.h"
#include "settingswindow.h"

#include <QComboBox>

static QMap<QString, QPair<QString, QString>> applications = {
    {QStringLiteral("Gear Editor"), {QStringLiteral("zone.xiv.novus.geareditor"), GEAREDITOR_EXECUTABLE}},
    {QStringLiteral("Map Editor"), {QStringLiteral("zone.xiv.novus.mapeditor"), MAPEDITOR_EXECUTABLE}},
    {QStringLiteral("Excel Editor"), {QStringLiteral("zone.xiv.novus.exceleditor"), EXCELEDITOR_EXECUTABLE}},
    {QStringLiteral("Data Explorer"), {QStringLiteral("zone.xiv.novus.dataexplorer"), DATAEXPLORER_EXECUTABLE}},
    {QStringLiteral("Enemy Editor"), {QStringLiteral("zone.xiv.novus.enemyeditor"), ENEMYEDITOR_EXECUTABLE}},
    {QStringLiteral("Patch Diff"), {QStringLiteral("zone.xiv.novus.patchdiff"), PATCHDIFF_EXECUTABLE}}};

static QMap<QString, QString> links = {{QStringLiteral("XIV Dev Wiki"), QStringLiteral("https://xiv.dev")},
                                       {QStringLiteral("XIV Docs"), QStringLiteral("https://docs.xiv.zone")}};

MainWindow::MainWindow()
{
    const auto appList = new QListWidget();

    const auto applicationHeader = new QListWidgetItem();
    applicationHeader->setText(i18nc("@title:group", "Applications"));
    applicationHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(applicationHeader);

    for (const auto &key : applications.keys()) {
        const auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(applications[key].first));

        appList->addItem(application);
    }

    const auto linksHeader = new QListWidgetItem();
    linksHeader->setText(i18nc("@title:group", "Links"));
    linksHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(linksHeader);

    for (const auto &key : links.keys()) {
        const auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(QStringLiteral("internet-web-browser")));

        appList->addItem(application);
    }

    connect(appList, &QListWidget::itemClicked, [](const QListWidgetItem *item) {
        if (applications.contains(item->text())) {
            const QString exec = applications[item->text()].second;
            QProcess::startDetached(exec, QStringList());
        } else if (links.contains(item->text())) {
            QDesktopServices::openUrl(QUrl::fromUserInput(links[item->text()]));
        }
    });

    const auto appListLayout = new QVBoxLayout();
    appListLayout->setContentsMargins(0, 0, 0, 0);
    appListLayout->addWidget(appList);

    const auto centralFrame = new QFrame();
    centralFrame->setContentsMargins(0, 0, 0, 0);
    centralFrame->setLayout(appListLayout);

    const auto formLayout = new QFormLayout();
    formLayout->setContentsMargins(5, 5, 5, 5); // TODO: use style values

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    m_gameInstallCombo = new QComboBox();
    connect(m_gameInstallCombo, &QComboBox::activated, this, [](const int index) {
        KConfig config(QStringLiteral("novusrc"));
        KConfigGroup game = config.group(QStringLiteral("Game"));

        auto gameInstalls = getGameInstalls();
        game.writeEntry(QStringLiteral("CurrentInstall"), gameInstalls[index].uuid.toString());

        config.sync();
    });
    m_gameInstallCombo->setMaximumWidth(175);
    formLayout->addRow(i18n("Game Install"), m_gameInstallCombo);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    const auto mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(centralFrame);
    mainLayout->addLayout(formLayout);
    const auto centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);

    setupActions();
    setupGUI(Create);

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    refreshGameInstalls();
}

void MainWindow::configure()
{
    const auto settingsWindow = new SettingsWindow();
    settingsWindow->show();

    // TODO: refresh game instals
}

void MainWindow::setupActions()
{
    KStandardAction::preferences(this, &MainWindow::configure, actionCollection());
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

void MainWindow::refreshGameInstalls() const
{
    m_gameInstallCombo->clear();

    int selectedIndex = 0;

    KConfig config(QStringLiteral("novusrc"));
    const KConfigGroup game = config.group(QStringLiteral("Game"));

    auto gameInstalls = getGameInstalls();
    for (const auto &install : gameInstalls) {
        if (install.uuid.toString() == game.readEntry(QStringLiteral("CurrentInstall"))) {
            selectedIndex = m_gameInstallCombo->count();
        }

        m_gameInstallCombo->addItem(install.label);
    }

    m_gameInstallCombo->setCurrentIndex(selectedIndex);
}

#include "moc_mainwindow.cpp"
