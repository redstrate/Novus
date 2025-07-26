// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

struct physis_InstanceObject;
class AppState;
class QLineEdit;

class ObjectPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectPropertiesWidget(AppState *appState, QWidget *parent = nullptr);

private:
    void resetObjectData();
    void refreshObjectData(const physis_InstanceObject &object);

    AppState *m_appState = nullptr;
    QLineEdit *m_idField = nullptr;
};
