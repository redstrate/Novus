// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
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

    const auto layout = new QVBoxLayout();
    setLayout(layout);

    const auto sourcesBox = new QGroupBox(i18n("Sources Output"));
    layout->addWidget(sourcesBox);

    const auto sourcesLayout = new QFormLayout();
    sourcesBox->setLayout(sourcesLayout);

    const auto sourcesBoxLayoutContainer = new QWidget(this);
    const auto sourcesBoxLayout = new QHBoxLayout(sourcesBoxLayoutContainer);
    sourcesBoxLayout->setContentsMargins(0, 0, 0, 0);

    KConfig config(QStringLiteral("novusrc"));
    const KConfigGroup game = config.group(QStringLiteral("Armoury"));

    m_sourcesLineEdit = new QLineEdit();
    m_sourcesLineEdit->setText(game.readEntry("SourcesOutputDirectory"));
    m_sourcesLineEdit->setClearButtonEnabled(true);
    sourcesBoxLayout->addWidget(m_sourcesLineEdit);

    const auto selectSourceUrlButton = new QPushButton(QIcon::fromTheme(QStringLiteral("folder-open")), QString());
    connect(selectSourceUrlButton, &QPushButton::clicked, this, [this] {
        const QUrl url = QFileDialog::getExistingDirectoryUrl(this, QString());
        if (!url.isEmpty()) {
            m_sourcesLineEdit->setText(url.toDisplayString(QUrl::PreferLocalFile));
        }
    });
    sourcesBoxLayout->addWidget(selectSourceUrlButton);

    sourcesLayout->addRow(i18n("Sources directory:"), sourcesBoxLayoutContainer);

    const auto penumbraBox = new QGroupBox(i18n("Penumbra Output"));
    layout->addWidget(penumbraBox);

    const auto penumbraLayout = new QFormLayout();
    penumbraBox->setLayout(penumbraLayout);

    const auto outputBoxLayoutContainer = new QWidget(this);
    const auto outputBoxLayout = new QHBoxLayout(outputBoxLayoutContainer);
    outputBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_outputLineEdit = new QLineEdit();
    m_outputLineEdit->setText(game.readEntry("PenumbraOutputDirectory"));
    m_outputLineEdit->setClearButtonEnabled(true);
    outputBoxLayout->addWidget(m_outputLineEdit);

    const auto selectHomeUrlButton = new QPushButton(QIcon::fromTheme(QStringLiteral("folder-open")), QString());
    connect(selectHomeUrlButton, &QPushButton::clicked, this, [this] {
        const QUrl url = QFileDialog::getExistingDirectoryUrl(this, QString());
        if (!url.isEmpty()) {
            m_outputLineEdit->setText(url.toDisplayString(QUrl::PreferLocalFile));
        }
    });
    outputBoxLayout->addWidget(selectHomeUrlButton);

    penumbraLayout->addRow(i18n("Output directory:"), outputBoxLayoutContainer);

    const auto blenderBox = new QGroupBox(i18n("Blender"));
    layout->addWidget(blenderBox);

    const auto blenderLayout = new QFormLayout();
    blenderBox->setLayout(blenderLayout);

    const auto blenderBoxLayoutContainer = new QWidget(this);
    const auto blenderBoxLayout = new QHBoxLayout(blenderBoxLayoutContainer);
    blenderBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_blenderPath = new QLineEdit();
    m_blenderPath->setText(game.readEntry("BlenderPath"));
    m_blenderPath->setClearButtonEnabled(true);
    blenderBoxLayout->addWidget(m_blenderPath);

    const auto selectBlenderButton = new QPushButton(QIcon::fromTheme(QStringLiteral("folder-open")), QString());
    connect(selectBlenderButton, &QPushButton::clicked, this, [this] {
        const QUrl url = QFileDialog::getOpenFileUrl(this, QString());
        if (!url.isEmpty()) {
            m_blenderPath->setText(url.toDisplayString(QUrl::PreferLocalFile));
        }
    });
    blenderBoxLayout->addWidget(selectBlenderButton);

    blenderLayout->addRow(i18n("Blender path:"), blenderBoxLayoutContainer);

    const auto bottomButtonLayout = new QHBoxLayout();
    layout->addLayout(bottomButtonLayout);

    const auto cancelButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-close")), i18n("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);
    bottomButtonLayout->addWidget(cancelButton);
    bottomButtonLayout->addStretch(1);

    const auto saveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Apply"));
    connect(saveButton, &QPushButton::clicked, this, [this] {
        applySettings();
        close();
    });
    bottomButtonLayout->addWidget(saveButton);
}

void SettingsWindow::applySettings() const
{
    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Armoury"));
    game.writeEntry("PenumbraOutputDirectory", m_outputLineEdit->text());
    game.writeEntry("SourcesOutputDirectory", m_sourcesLineEdit->text());
    game.writeEntry("BlenderPath", m_blenderPath->text());
}
