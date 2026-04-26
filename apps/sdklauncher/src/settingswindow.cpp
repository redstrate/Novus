// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingswindow.h"

#include "settings.h"
#include <physis.hpp>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPageView>
#include <KPageWidgetModel>
#include <QFileDialog>
#include <QFormLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class GameInstallPage : public QWidget
{
public:
    explicit GameInstallPage(QWidget *parent)
        : QWidget(parent)
    {
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

        m_languageEdit = new EnumEdit<Language>();
        connect(m_languageEdit, &EnumEdit<Language>::editingFinished, this, [this] {
            auto &install = currentInstall();
            install.language = m_languageEdit->value();
        });
        configureWidgetLayout->addRow(i18n("Language:"), m_languageEdit);

        // TODO: port to QDialogButtonBox
        auto bottomButtonLayout = new QHBoxLayout();
        layout->addLayout(bottomButtonLayout);

        m_addButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add"));
        connect(m_addButton, &QPushButton::clicked, this, &GameInstallPage::addInstall);
        bottomButtonLayout->addWidget(m_addButton);

        m_removeButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove"));
        connect(m_removeButton, &QPushButton::clicked, this, &GameInstallPage::removeInstall);
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

private:
    void applySettings()
    {
        saveGameInstalls(m_installs);
    }

    void refreshList()
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

    void refreshConfigureWidget(const QUuid uuid)
    {
        bool valid = false;
        for (const auto &install : m_installs) {
            if (install.uuid == uuid) {
                m_labelEdit->setText(install.label);
                m_pathEdit->setText(install.path);
                m_languageEdit->setValue(install.language);

                valid = true;
            }
        }

        m_labelEdit->setEnabled(valid);
        m_pathEdit->setEnabled(valid);
        m_languageEdit->setEnabled(valid);
        m_removeButton->setEnabled(m_installs.size() > 1);
    }

    void addInstall()
    {
        addNewInstall();
        refreshList();
    }

    void removeInstall()
    {
        m_installs.pop_back();
        refreshList();
    }

    GameInstall &currentInstall()
    {
        const auto row = m_installWidget->currentRow();
        return m_installs[row];
    }

    QListWidget *m_installWidget;
    QLineEdit *m_labelEdit;
    QLineEdit *m_pathEdit;
    EnumEdit<Language> *m_languageEdit;
    QList<GameInstall> m_installs;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
};

class ModPage : public QWidget
{
public:
    explicit ModPage(QWidget *parent)
        : QWidget(parent)
    {
        auto layout = new QVBoxLayout();
        setLayout(layout);

        auto sideLayout = new QHBoxLayout();
        layout->addLayout(sideLayout);

        m_modsWidget = new QListWidget();
        connect(m_modsWidget, &QListWidget::activated, this, [this] {
            const auto row = m_modsWidget->currentRow();
            refreshConfigureWidget(m_mods[row].uuid);
        });
        sideLayout->addWidget(m_modsWidget);

        auto configureWidget = new QWidget();
        sideLayout->addWidget(configureWidget);

        auto configureWidgetLayout = new QFormLayout();
        configureWidget->setLayout(configureWidgetLayout);

        m_pathEdit = new QLineEdit();
        connect(m_pathEdit, &QLineEdit::editingFinished, this, [this] {
            auto &install = currentMod();
            install.path = m_pathEdit->text();
        });
        configureWidgetLayout->addRow(i18n("Path:"), m_pathEdit);

        // TODO: port to QDialogButtonBox
        auto bottomButtonLayout = new QHBoxLayout();
        layout->addLayout(bottomButtonLayout);

        m_addButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add"));
        connect(m_addButton, &QPushButton::clicked, this, &ModPage::addMod);
        bottomButtonLayout->addWidget(m_addButton);

        m_removeButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove"));
        connect(m_removeButton, &QPushButton::clicked, this, &ModPage::removeMod);
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

private:
    void applySettings()
    {
        saveGameMods(m_mods);
    }

    void refreshList()
    {
        m_modsWidget->clear();

        m_mods = getGameMods();
        for (const auto &install : m_mods) {
            m_modsWidget->addItem(install.uuid.toString());
        }

        if (!m_mods.isEmpty()) {
            refreshConfigureWidget(m_mods.constFirst().uuid);
            m_modsWidget->setCurrentRow(0);
        }
    }

    void refreshConfigureWidget(const QUuid uuid)
    {
        bool valid = false;
        for (const auto &install : m_mods) {
            if (install.uuid == uuid) {
                m_pathEdit->setText(install.path);

                valid = true;
            }
        }

        m_pathEdit->setEnabled(valid);
        m_removeButton->setEnabled(m_mods.size() > 1);
    }

    void addMod()
    {
        addNewGameMod();
        refreshList();
    }

    void removeMod()
    {
        m_mods.pop_back();
        refreshList();
    }

    GameMod &currentMod()
    {
        const auto row = m_modsWidget->currentRow();
        return m_mods[row];
    }

    QListWidget *m_modsWidget;
    QLineEdit *m_pathEdit;
    QList<GameMod> m_mods;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
};

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18n("Settings"));

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    auto pageModel = new KPageWidgetModel();

    auto pageView = new KPageView();
    pageView->setModel(pageModel);
    layout->addWidget(pageView);

    auto gameInstallPageWidget = new GameInstallPage(this);
    pageModel->addPage(gameInstallPageWidget, i18n("Game"))->setIcon(QIcon::fromTheme(QStringLiteral("applications-games-symbolic")));

    auto modPageWidget = new ModPage(this);
    pageModel->addPage(modPageWidget, i18n("Mods"))->setIcon(QIcon::fromTheme(QStringLiteral("kt-plugins-symbolic")));
}
