// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "editwidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT OpenPathHandler : public QObject
{
    Q_OBJECT
public:
    void openPath(const QString &path);

    void setEmitSignal(bool emit);

Q_SIGNALS:
    void pathOpened(const QString &path);

private:
    bool m_emitSignal = false;
};

/// Represents a game path, and allows you to perform additional actions e.g. opening said path in Data Explorer.
class NOVUSCOMMON_EXPORT PathEdit : public EditWidget
{
    Q_OBJECT

public:
    explicit PathEdit(QWidget *parent = nullptr);
    ~PathEdit() override = default;

    void setPath(const QString &path);
    void setReadOnly(bool readOnly);

    static OpenPathHandler *handler();

private:
    QLineEdit *m_lineEdit = nullptr;
    QPushButton *m_openButton = nullptr;
};
