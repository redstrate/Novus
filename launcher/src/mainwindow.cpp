// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KConfig>
#include <KConfigGroup>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QListWidget>
#include <QProcess>
#include <QVBoxLayout>

static QMap<QString, QString> applications = {{QStringLiteral("Armoury - View and export gear"), QStringLiteral("novus-armoury")},
                                              {QStringLiteral("Karuku - Read Excel sheets"), QStringLiteral("novus-karuku")},
                                              {QStringLiteral("Sagasu - Explore data archives"), QStringLiteral("novus-sagasu")},
                                              {QStringLiteral("Model Viewer - Preview MDL files"), QStringLiteral("novus-mdlviewer")}};

MainWindow::MainWindow()
{
    setWindowTitle(QStringLiteral("Novus SDK"));

    auto appList = new QListWidget();

    auto applicationHeader = new QListWidgetItem();
    applicationHeader->setText(QStringLiteral("Applications"));
    applicationHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(applicationHeader);

    for (const auto &name : applications.keys()) {
        appList->addItem(name);
    }

    connect(appList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        const QString exec = QStringLiteral("./") + applications[item->text()];

        qDebug() << "Launching" << exec;

        QProcess::startDetached(exec, QStringList());
    });

    auto appListLayout = new QVBoxLayout();
    appListLayout->addWidget(appList);

    auto centralFrame = new QFrame();
    centralFrame->setLayout(appListLayout);

    auto formLayout = new QFormLayout();

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group("Game");

    auto gameCombo = new QComboBox();
    formLayout->addRow(QStringLiteral("Current Game"), gameCombo);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    gameCombo->addItem(game.readEntry("GameDir"));

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(centralFrame);
    mainLayout->addLayout(formLayout);
    auto centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
}
