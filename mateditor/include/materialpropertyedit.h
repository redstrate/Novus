// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <QTabWidget>
#include <physis.hpp>

class MaterialPropertyEdit : public QWidget
{
    Q_OBJECT

public:
    explicit MaterialPropertyEdit(GameData *data, QWidget *parent = nullptr);

    void setMaterial(physis_Material material);

private:
    void rebuild();

    QVBoxLayout *m_itemsLayout = nullptr;
    QLineEdit *m_shaderPackageName = nullptr;

    QTabWidget *m_tabWidget = nullptr;
    QVBoxLayout *m_propertiesLayout = nullptr;
    QVBoxLayout *m_texturesLayout = nullptr;

    physis_Material m_material = {};
    physis_SHPK m_shpk = {};

    GameData *m_data = nullptr;
};