// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <physis.hpp>

class MaterialPropertyEdit : public QWidget
{
    Q_OBJECT

public:
    explicit MaterialPropertyEdit(QWidget *parent = nullptr);

    void setMaterial(physis_Material material);

private:
    void rebuild();

    QVBoxLayout *m_itemsLayout = nullptr;
    QLineEdit *m_shaderPackageName = nullptr;

    physis_Material m_material;
};
