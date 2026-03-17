// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exceledit.h"

#include "../exd/excelmodel.h"
#include "excelmodel.h"
#include "excelresolver.h"
#include "scenestate.h"
#include "schema.h"
#include "settings.h"

#include <QDir>
#include <QHBoxLayout>
#include <QStandardPaths>

ExcelEdit::ExcelEdit(SceneState *state, const QStringList &excelSheets, uint32_t &rowId, QWidget *parent)
    : QWidget(parent)
    , m_rowId(rowId)
{
    auto layout = new QHBoxLayout(this);
    setMaximumHeight(35); // FIXME: don't hard-code
    layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    layout->addWidget(m_lineEdit);

    auto resolver = new CachingExcelResolver(state->resource());

    m_models.reserve(excelSheets.size());
    m_sheets.reserve(excelSheets.size());
    for (const auto &sheetName : excelSheets) {
        const std::string exhName = sheetName.toLower().toStdString();

        const auto exhFile = physis_sqpack_read(state->resource(), (std::string("exd/") + exhName + ".exh").c_str());
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/" << sheetName << ".exh";
        } else {
            const auto exh = physis_exh_parse(state->resource()->platform, exhFile);
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
                auto sheet = physis_sqpack_read_excel_sheet(state->resource(), sheetName.toStdString().c_str(), &exh, language);
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

    for (const auto &[name, model] : m_models) {
        if (const auto display = model->resolveDisplay(rowId); !display.isNull()) {
            m_lineEdit->setText(QStringLiteral("%1 (%2#%3)").arg(display.toString()).arg(name).arg(rowId));
            return;
        }

        // As a fallback, check if it exists on the sheet. Handles cases like ENpcBase which technically doesn't have a display field.
        if (model->existsOnSheet(rowId)) {
            m_lineEdit->setText(QStringLiteral("%1#%2").arg(name).arg(rowId));
            return;
        }
    }

    m_lineEdit->setText(QStringLiteral("???#%1").arg(rowId));
}

#include "moc_exceledit.cpp"
