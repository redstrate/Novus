// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTableWidget>
#include <QWidget>
#include <physis.hpp>

class QTextEdit;

class TmbPart : public QWidget
{
    Q_OBJECT

public:
    explicit TmbPart(physis_SqPackResource *resource, QWidget *parent = nullptr);

    void load(physis_Buffer file);
    void loadExisting(physis_Tmb tmb);

private:
    physis_Tmb m_tmb = {};
    physis_SqPackResource *m_resource = nullptr;
};
