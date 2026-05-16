// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <KXmlGuiWindow>
#include <QFormLayout>

class QTableView;
class MDLPart;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);

private:
    void setupActions();

    MDLPart *m_part = nullptr;
    FileCache m_cache;
    QFormLayout *m_detailsLayout = nullptr;
    QTableView *m_tableView;
};
