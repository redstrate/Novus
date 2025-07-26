// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "mdlpart.h"
#include <QComboBox>
#include <QWidget>
#include <physis.hpp>

struct SqPackResource;

class MaterialView : public QWidget
{
    Q_OBJECT

public:
    explicit MaterialView(SqPackResource *data, FileCache &cache, QWidget *parent = nullptr);

    MDLPart &part() const;

public Q_SLOTS:
    void addSphere(physis_Material material);

private:
    MDLPart *mdlPart = nullptr;

    SqPackResource *data;
    FileCache &cache;

    physis_MDL m_mdl;
};
