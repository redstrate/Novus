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

    SingleGearView *gearView = nullptr;
    FullModelViewer *fullModelViewer = nullptr;
    QTabWidget *materialsView = nullptr;
    MetadataView *metadataView = nullptr;

    physis_SqPackResource m_data;
    FileCache cache;
    PenumbraApi *m_api = nullptr;
};
