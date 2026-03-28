// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exceledit.h"

#include "excelmodel.h"
#include "excelresolver.h"
#include "filecache.h"
#include "launcherconfig.h"
#include "scenestate.h"
#include "schema.h"
#include "settings.h"

#include <KLocalizedString>
#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QProcess>
#include <QStandardPaths>

ExcelEdit::ExcelEdit(SceneState *state, const QStringList &excelSheets, uint32_t &rowId, QWidget *parent)
    : QWidget(parent)
    , m_rowId(rowId)
{
    auto layout = new QHBoxLayout(this);
    setMaximumHeight(35); // FIXME: don't hard-code
    layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    m_lineEdit->setReadOnly(true);
    layout->addWidget(m_lineEdit);

    auto resolver = new CachingExcelResolver(&state->cache().resource());

    m_models.reserve(excelSheets.size());
    m_sheets.reserve(excelSheets.size());
    for (const auto &sheetName : excelSheets) {
        const std::string exhName = sheetName.toLower().toStdString();

        const auto exhFile = physis_sqpack_read(&state->cache().resource(), (std::string("exd/") + exhName + ".exh").c_str());
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/" << sheetName << ".exh";
        } else {
            const auto exh = physis_exh_parse(state->cache().resource().platform, exhFile);
            if (!exh.p_ptr) {
                qWarning() << "Failed to parse exd/" << sheetName << ".exh";
            } else {
                auto language = Language::None;
                const Language desiredLanguage = getLanguage();
                for (uint32_t i = 0; i < exh.language_count; i++) {
                    if (exh.languages[i] == desiredLanguage) {
                        language = desiredLanguage;
                        break;
                    }
                }
                auto sheet = physis_sqpack_read_excel_sheet(&state->cache().resource(), sheetName.toStdString().c_str(), &exh, language);
                m_sheets.push_back(sheet);

                const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                const QDir schemaDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

                const Schema schema(schemaDir.absoluteFilePath(QStringLiteral("%1.yml").arg(sheetName)));

                for (uint32_t i = 0; i < sheet.page_count; i++) {
                    m_models.push_back({sheetName, new ExcelModel(exh, sheet.pages[i], schema, resolver, language)});
                }
            }
        }
    }

    auto goToButton = new QPushButton();
    goToButton->setIcon(QIcon::fromTheme(QStringLiteral("overflow-menu")));
    layout->addWidget(goToButton);

    m_menu = new QMenu();
    goToButton->setMenu(m_menu);

    updateRow();
}

void ExcelEdit::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    updateRow();
}

void ExcelEdit::updateRow()
{
    m_lineEdit->clear();
    m_menu->clear();

    if (!m_readOnly) {
        auto editAction = m_menu->addAction(i18n("Edit…"));
        connect(editAction, &QAction::triggered, this, [this] {
            m_rowId = QInputDialog::getInt(this, i18n("Enter Row ID"), i18n("Row ID:"), m_rowId, 0, std::numeric_limits<int>::max());
            updateRow();
        });

        m_menu->addSeparator();
    }

    for (const auto &[name, model] : m_models) {
        const auto addResolveAction = [this, name] {
            auto resolveAction = m_menu->addAction(i18n("Go to %1…").arg(name));
            connect(resolveAction, &QAction::triggered, this, [this, name] {
                QProcess::startDetached(EXCELEDITOR_EXECUTABLE, {QStringLiteral("%1#%2").arg(name).arg(m_rowId)});
            });
        };

        if (const auto display = model->resolveDisplay(m_rowId); !display.isNull()) {
            if (m_lineEdit->text().isEmpty()) {
                m_lineEdit->setText(QStringLiteral("%1 (%2#%3)").arg(display.toString()).arg(name).arg(m_rowId));
            }
            addResolveAction();
        } else {
            // As a fallback, check if it exists on the sheet. Handles cases like ENpcBase which technically doesn't have a display field.
            if (model->existsOnSheet(m_rowId)) {
                if (m_lineEdit->text().isEmpty()) {
                    m_lineEdit->setText(QStringLiteral("%1#%2").arg(name).arg(m_rowId));
                }
                addResolveAction();
            }
        }
    }

    if (m_lineEdit->text().isEmpty()) {
        m_lineEdit->setText(QStringLiteral("???#%1").arg(m_rowId));
    }
}

#include "moc_exceledit.cpp"
