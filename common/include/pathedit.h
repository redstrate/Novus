// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "novuscommon_export.h"

/// Represents a game path, and allows you to perform additional actions e.g. opening said path in Data Explorer.
class NOVUSCOMMON_EXPORT PathEdit : public QWidget
{
    Q_OBJECT

public:
    explicit PathEdit(QWidget *parent = nullptr);

    void setPath(const QString &path);
    void setReadOnly(bool readOnly);

private:
    QLineEdit *m_lineEdit = nullptr;
    QPushButton *m_openButton = nullptr;
};
