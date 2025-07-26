// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"

struct SqPackResource;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SqPackResource *data);

private:
    void setupActions();

    SqPackResource *data = nullptr;
    FileCache cache;
    physis_Material m_material;
};
