// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"
#include "hashdatabase.h"

class HexPart;
class KRecentFilesMenu;
class DiffTreeWidget;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);
    ~MainWindow() override;

private:
    void setupActions();
    void openPatch(const QUrl &url);

    physis_SqPackResource m_data;
    FileCache cache;
    HashDatabase m_database;
    DiffTreeWidget *m_diffTreeWidget = nullptr;
    KRecentFilesMenu *m_recentFilesMenu = nullptr;
    HexPart *m_hexPart = nullptr;
};
