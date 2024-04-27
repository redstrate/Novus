// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <QTabWidget>
#include <physis.hpp>

class MtrlPart : public QWidget
{
    Q_OBJECT

public:
    explicit MtrlPart(GameData *data, QWidget *parent = nullptr);

    void load(physis_Material file);

private:
    void rebuild();

    QVBoxLayout *m_itemsLayout = nullptr;
    QLineEdit *m_shaderPackageName = nullptr;

    QTabWidget *m_tabWidget = nullptr;
    QVBoxLayout *m_propertiesLayout = nullptr;
    QVBoxLayout *m_texturesLayout = nullptr;
    QVBoxLayout *m_constantsLayout = nullptr;

    physis_Material m_material = {};
    physis_SHPK m_shpk = {};

    GameData *m_data = nullptr;
};