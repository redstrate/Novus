#include "settingswindow.h"

#include <KConfig>
#include <KConfigGroup>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Settings"));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto penumbraBox = new QGroupBox(QStringLiteral("Penumbra Output"));
    layout->addWidget(penumbraBox);

    auto penumbraLayout = new QFormLayout();
    penumbraBox->setLayout(penumbraLayout);

    auto outputBoxLayoutContainer = new QWidget(this);
    auto outputBoxLayout = new QHBoxLayout(outputBoxLayoutContainer);
    outputBoxLayout->setContentsMargins(0, 0, 0, 0);

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group("Armoury");

    m_outputLineEdit = new QLineEdit();
    m_outputLineEdit->setText(game.readEntry("PenumbraOutputDirectory"));
    m_outputLineEdit->setClearButtonEnabled(true);
    outputBoxLayout->addWidget(m_outputLineEdit);

    auto selectHomeUrlButton = new QPushButton(QIcon::fromTheme(QStringLiteral("folder-open")), QString());
    connect(selectHomeUrlButton, &QPushButton::clicked, this, [this] {
        QUrl url = QFileDialog::getExistingDirectoryUrl(this, QString());
        if (!url.isEmpty()) {
            m_outputLineEdit->setText(url.toDisplayString(QUrl::PreferLocalFile));
        }
    });
    outputBoxLayout->addWidget(selectHomeUrlButton);

    penumbraLayout->addRow(QStringLiteral("Output Directory"), outputBoxLayoutContainer);

    auto bottomButtonLayout = new QHBoxLayout();
    layout->addLayout(bottomButtonLayout);

    auto cancelButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-close")), QStringLiteral("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);
    bottomButtonLayout->addWidget(cancelButton);
    bottomButtonLayout->addStretch(1);

    auto saveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-ok")), QStringLiteral("Apply"));
    connect(saveButton, &QPushButton::clicked, this, [this] {
        applySettings();
        close();
    });
    bottomButtonLayout->addWidget(saveButton);
}

void SettingsWindow::applySettings()
{
    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group("Armoury");
    game.writeEntry("PenumbraOutputDirectory", m_outputLineEdit->text());
}
