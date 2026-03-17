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
    explicit ExcelEdit(SceneState *state, const QStringList &excelSheets, uint32_t &rowId, QWidget *parent = nullptr);

    void setReadOnly(bool readOnly);

private:
    void updateRow();

    QLineEdit *m_lineEdit = nullptr;
    uint32_t &m_rowId;
    QList<physis_ExcelSheet> m_sheets;
    QList<std::pair<QString, ExcelModel *>> m_models;
    QMenu *m_menu = nullptr;
    bool m_readOnly = false;
};
