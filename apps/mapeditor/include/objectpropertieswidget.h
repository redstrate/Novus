// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QVBoxLayout;
struct physis_InstanceObject;
struct physis_BGInstanceObject;
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

    void addCommonSection(const physis_InstanceObject &object);
    void addBGSection(const physis_BGInstanceObject &bg);

    AppState *m_appState = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QWidget *> m_sections;
};
