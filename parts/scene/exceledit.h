// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <physis.hpp>

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class ExcelModel;
class SceneState;

class ExcelEdit : public QWidget
{
    Q_OBJECT

public:
    explicit ExcelEdit(SceneState *state, const QString &excelSheet, QWidget *parent = nullptr);

    void setRowId(uint32_t id);

private:
    QLineEdit *m_lineEdit = nullptr;
    uint32_t m_rowId = 0;
    physis_ExcelSheet m_sheet;
    QString m_excelSheet;
    ExcelModel *m_model = nullptr;
};
