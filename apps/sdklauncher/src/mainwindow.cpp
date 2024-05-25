// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFormLayout>
#include <QListWidget>
#include <QProcess>
#include <QUrl>
#include <QVBoxLayout>

#include "launcherconfig.h"

static QMap<QString, QPair<QString, QString>> applications = {
    {QStringLiteral("Gear Editor"), {QStringLiteral("zone.xiv.armoury"), GEAREDITOR_EXECUTABLE}},
    {QStringLiteral("Map Editor"), {QStringLiteral("zone.xiv.mapeditor"), MAPEDITOR_EXECUTABLE}},
    {QStringLiteral("Material Editor"), {QStringLiteral("zone.xiv.mateditor"), MATEDITOR_EXECUTABLE}},
    {QStringLiteral("Excel Editor"), {QStringLiteral("zone.xiv.karaku"), EXCELEDITOR_EXECUTABLE}},
    {QStringLiteral("Data Explorer"), {QStringLiteral("zone.xiv.sagasu"), DATAEXPLORER_EXECUTABLE}},
    {QStringLiteral("Model Viewer"), {QStringLiteral("zone.xiv.mdlviewer"), MDLVIEWER_EXECUTABLE}},
    {QStringLiteral("Game Launcher"), {QStringLiteral("zone.xiv.gamelauncher"), GAMELAUNCHER_EXECUTABLE}}};

static QMap<QString, QString> links = {{QStringLiteral("XIV Dev Wiki"), QStringLiteral("https://xiv.dev")},
                                       {QStringLiteral("XIV Docs"), QStringLiteral("https://docs.xiv.zone")}};

MainWindow::MainWindow()
    : NovusMainWindow()
{
    setupMenubar();

    auto appList = new QListWidget();

    auto applicationHeader = new QListWidgetItem();
    applicationHeader->setText(i18nc("@title:group", "Applications"));
    applicationHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(applicationHeader);

    for (const auto &key : applications.keys()) {
        auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(applications[key].first));

        appList->addItem(application);
    }

    auto linksHeader = new QListWidgetItem();
    linksHeader->setText(i18nc("@title:group", "Links"));
    linksHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(linksHeader);

    for (const auto &key : links.keys()) {
        auto application = new QListWidgetItem();
        application->setText(key);
        application->setIcon(QIcon::fromTheme(QStringLiteral("internet-web-browser")));

        appList->addItem(application);
    }

    connect(appList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        if (applications.contains(item->text())) {
            const QString exec = QStringLiteral("./") + applications[item->text()].second;

            qDebug() << "Launching" << exec;

            QProcess::startDetached(exec, QStringList());
        } else if (links.contains(item->text())) {
            QDesktopServices::openUrl(QUrl::fromUserInput(links[item->text()]));
        }
    });

    auto appListLayout = new QVBoxLayout();
    appListLayout->setContentsMargins(0, 0, 0, 0);
    appListLayout->addWidget(appList);

    auto centralFrame = new QFrame();
    centralFrame->setContentsMargins(0, 0, 0, 0);
    centralFrame->setLayout(appListLayout);

    auto formLayout = new QFormLayout();
    formLayout->setContentsMargins(5, 5, 5, 5); // TODO: use style values

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    auto gameCombo = new QComboBox();
    gameCombo->setMaximumWidth(175);
    formLayout->addRow(i18n("Current game"), gameCombo);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    gameCombo->addItem(game.readEntry("GameDir"));

    auto mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(centralFrame);
    mainLayout->addLayout(formLayout);
    auto centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
}

#include "moc_mainwindow.cpp"