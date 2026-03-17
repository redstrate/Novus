// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exceledit.h"

#include "excelmodel.h"
#include "excelresolver.h"
#include "scenestate.h"
#include "schema.h"
#include "settings.h"

#include <QDir>
#include <QHBoxLayout>
#include <QStandardPaths>

ExcelEdit::ExcelEdit(SceneState *state, const QString &excelSheet, QWidget *parent)
    : QWidget(parent)
    , m_excelSheet(excelSheet)
{
    auto layout = new QHBoxLayout(this);
    setMaximumHeight(35); // FIXME: don't hard-code
    layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    layout->addWidget(m_lineEdit);

    const std::string exhName = excelSheet.toLower().toStdString();

    const auto exhFile = physis_sqpack_read(state->resource(), (std::string("exd/") + exhName + ".exh").c_str());
    if (exhFile.size == 0) {
        qWarning() << "Failed to read exd/" << excelSheet << ".exh";
    } else {
        const auto exh = physis_exh_parse(state->resource()->platform, exhFile);
        if (!exh.p_ptr) {
            qWarning() << "Failed to parse exd/" << excelSheet << ".exh";
        } else {
            m_sheet = physis_sqpack_read_excel_sheet(state->resource(), excelSheet.toStdString().c_str(), &exh, getLanguage());

            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            const QDir schemaDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

            const Schema schema(schemaDir.absoluteFilePath(QStringLiteral("%1.yml").arg(excelSheet)));

            // TODO: make this more efficient, don't initialize this every time
            auto resolver = new CachingExcelResolver(state->resource());
            m_model = new ExcelModel(exh, m_sheet.pages[0], schema, resolver, getLanguage());
        }
    }
}

void ExcelEdit::setRowId(const uint32_t rowId)
{
    m_rowId = rowId;
    m_lineEdit->setText(m_model->resolveDisplay(rowId).toString());
}

#include "moc_exceledit.cpp"
