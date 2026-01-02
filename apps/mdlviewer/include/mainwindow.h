// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <KXmlGuiWindow>
#include <QFormLayout>

class MDLPart;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);

private:
    void setupActions();

    physis_SqPackResource m_data;
    MDLPart *part = nullptr;
    FileCache cache;
    QFormLayout *m_detailsLayout = nullptr;
};
