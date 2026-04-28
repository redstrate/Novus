// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"
#include "hashdatabase.h"

class DiffTreeWidget;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);
    ~MainWindow() override;

private:
    void setupActions();

    physis_SqPackResource m_data;
    FileCache cache;
    HashDatabase m_database;
    DiffTreeWidget *m_diffTreeWidget = nullptr;
};
