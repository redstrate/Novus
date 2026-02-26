// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingswindow.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18n("Settings"));

    auto layout = new QFormLayout();
    setLayout(layout);

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("MapEditor"));

    m_dropinsLineEdit = new QLineEdit();
    m_dropinsLineEdit->setText(game.readEntry("DropInsPath"));
    m_dropinsLineEdit->setClearButtonEnabled(true);
    layout->addRow(i18n("Drop-ins"), m_dropinsLineEdit);

    auto bottomButtonLayout = new QHBoxLayout();
    layout->addRow(bottomButtonLayout);

    auto cancelButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-close")), i18n("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);
    bottomButtonLayout->addWidget(cancelButton);
    bottomButtonLayout->addStretch(1);

    auto saveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Apply"));
    connect(saveButton, &QPushButton::clicked, this, [this] {
        applySettings();
        close();
    });
    bottomButtonLayout->addWidget(saveButton);
}

void SettingsWindow::applySettings()
{
    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("MapEditor"));
    game.writeEntry("DropInsPath", m_dropinsLineEdit->text());
}
