// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class SceneState;

class ObjectIdEdit : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectIdEdit(SceneState *state, QWidget *parent = nullptr);

    void setObjectId(uint32_t id);

private:
    QLineEdit *m_lineEdit = nullptr;
    QPushButton *m_goToButton = nullptr;
    uint32_t m_objectId = 0;
};
