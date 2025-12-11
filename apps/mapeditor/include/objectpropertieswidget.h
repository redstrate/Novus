// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

struct physis_GameInstanceObject;
struct physis_NPCInstanceObject;
struct physis_TriggerBoxInstanceObject;
struct physis_MapRangeInstanceObject;
struct physis_Layer;
struct physis_PopRangeInstanceObject;
struct physis_EventInstanceObject;
class QVBoxLayout;
struct physis_InstanceObject;
struct physis_BGInstanceObject;
class AppState;
class QLineEdit;
struct physis_ENPCInstanceObject;

class ObjectPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectPropertiesWidget(AppState *appState, QWidget *parent = nullptr);

private:
    void resetSections();
    void refreshObjectData(const physis_InstanceObject &object);
    void refreshLayerData(const physis_Layer &layer);

    void addCommonSection(const physis_InstanceObject &object);
    void addBGSection(const physis_BGInstanceObject &bg);
    void addEventSection(const physis_EventInstanceObject &eobj);
    void addPopRangeSection(const physis_PopRangeInstanceObject &pop);
    void addEventNPCSection(const physis_ENPCInstanceObject &enpc);
    void addMapRangeSection(const physis_MapRangeInstanceObject &mapRange);
    void addTriggerBoxSection(const physis_TriggerBoxInstanceObject &triggerBox);
    void addNPCSection(const physis_NPCInstanceObject &npc);
    void addGameObjectSection(const physis_GameInstanceObject &object);

    AppState *m_appState = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QWidget *> m_sections;
};
