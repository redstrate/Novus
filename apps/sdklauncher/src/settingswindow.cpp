// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingswindow.h"

#include "settings.h"
#include <physis.hpp>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QFileDialog>
#include <QFormLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18n("Settings"));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto sideLayout = new QHBoxLayout();
    layout->addLayout(sideLayout);

    m_installWidget = new QListWidget();
    connect(m_installWidget, &QListWidget::activated, this, [this] {
        const auto row = m_installWidget->currentRow();
        refreshConfigureWidget(m_installs[row].uuid);
    });
    sideLayout->addWidget(m_installWidget);

    auto configureWidget = new QWidget();
    sideLayout->addWidget(configureWidget);

    auto configureWidgetLayout = new QFormLayout();
    configureWidget->setLayout(configureWidgetLayout);

    m_labelEdit = new QLineEdit();
    connect(m_labelEdit, &QLineEdit::editingFinished, this, [this] {
        auto &install = currentInstall();
        install.label = m_labelEdit->text();
    });
    configureWidgetLayout->addRow(i18n("Label:"), m_labelEdit);

    m_pathEdit = new QLineEdit();
    connect(m_pathEdit, &QLineEdit::editingFinished, this, [this] {
        auto &install = currentInstall();
        install.path = m_pathEdit->text();
    });
    configureWidgetLayout->addRow(i18n("Game Path:"), m_pathEdit);

    // TODO: port to QDialogButtonBox
    auto bottomButtonLayout = new QHBoxLayout();
    layout->addLayout(bottomButtonLayout);

    m_addButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add"));
    connect(m_addButton, &QPushButton::clicked, this, &SettingsWindow::addInstall);
    bottomButtonLayout->addWidget(m_addButton);

    m_removeButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove"));
    connect(m_removeButton, &QPushButton::clicked, this, &SettingsWindow::removeInstall);
    bottomButtonLayout->addWidget(m_removeButton);
    bottomButtonLayout->addStretch(1);

    auto cancelButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-close")), i18n("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);
    bottomButtonLayout->addWidget(cancelButton);

    auto saveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Apply"));
    connect(saveButton, &QPushButton::clicked, this, [this] {
        applySettings();
        close();
    });
    bottomButtonLayout->addWidget(saveButton);

    refreshList();
}

void SettingsWindow::applySettings()
{
    saveGameInstalls(m_installs);
}

void SettingsWindow::refreshList()
{
    m_installWidget->clear();

    m_installs = getGameInstalls();
    for (const auto &install : m_installs) {
        m_installWidget->addItem(install.label);
    }

    if (!m_installs.isEmpty()) {
        refreshConfigureWidget(m_installs.constFirst().uuid);
        m_installWidget->setCurrentRow(0);
    }
}

void SettingsWindow::refreshConfigureWidget(const QUuid uuid)
{
    bool valid = false;
    for (const auto &install : m_installs) {
        if (install.uuid == uuid) {
            m_labelEdit->setText(install.label);
            m_pathEdit->setText(install.path);

            valid = true;
        }
    }

    m_labelEdit->setEnabled(valid);
    m_pathEdit->setEnabled(valid);
    m_removeButton->setEnabled(m_installs.size() > 1);
}

void SettingsWindow::addInstall()
{
    addNewInstall();
    refreshList();
}

void SettingsWindow::removeInstall()
{
    m_installs.pop_back();
    refreshList();
}

GameInstall &SettingsWindow::currentInstall()
{
    const auto row = m_installWidget->currentRow();
    return m_installs[row];
}
