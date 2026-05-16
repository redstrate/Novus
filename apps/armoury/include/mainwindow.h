// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <physis.hpp>

#include "fullmodelviewer.h"
#include "gearview.h"
#include "metadataview.h"
#include "mtrlpart.h"
#include "singlegearview.h"

class FileCache;
class PenumbraApi;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);

public Q_SLOTS:
    void configure();

private:
    void setupActions();

    SingleGearView *m_gearView = nullptr;
    FullModelViewer *m_fullModelViewer = nullptr;
    QTabWidget *m_materialsView = nullptr;
    MetadataView *m_metadataView = nullptr;

    FileCache m_cache;
    PenumbraApi *m_api = nullptr;
};
