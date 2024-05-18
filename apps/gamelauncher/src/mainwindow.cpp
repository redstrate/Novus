// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "settings.h"

#include <KLocalizedString>
#include <QApplication>
#include <QListWidget>
#include <QMenuBar>
#include <QFormLayout>
#include <QPushButton>
#include <QProcess>
#include <QLineEdit>

MainWindow::MainWindow() : NovusMainWindow()
{
    setMinimumSize(1280, 720);
    setupMenubar();

    process = new QProcess();
    process->setWorkingDirectory(getGameDirectory());
    process->setProgram(QStringLiteral("ffxiv_dx11.exe"));

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QFormLayout();
    dummyWidget->setLayout(layout);

    auto argsBox = new QLineEdit();
    layout->addRow(i18n("Arguments"), argsBox);

    auto launchGameButton = new QPushButton(i18n("Launch Game"));
    connect(launchGameButton, &QPushButton::clicked, this, [this, argsBox] {
        process->setArguments(argsBox->text().split(QLatin1Char(' ')));
        qInfo() << "Launching" << process->program() << process->arguments();
        process->start();
    });
    layout->addWidget(launchGameButton);
}

#include "moc_mainwindow.cpp"