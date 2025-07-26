// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QComboBox>
#include <physis.hpp>
#include <unordered_map>

#include "fullmodelviewer.h"
#include "gearview.h"
#include "metadataview.h"
#include "mtrlpart.h"
#include "singlegearview.h"

struct SqPackResource;
class FileCache;
class PenumbraApi;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SqPackResource *data);

public Q_SLOTS:
    void configure();

private:
    void setupActions();

    SingleGearView *gearView = nullptr;
    FullModelViewer *fullModelViewer = nullptr;
    QTabWidget *materialsView = nullptr;
    MetadataView *metadataView = nullptr;

    SqPackResource &data;
    FileCache cache;
    PenumbraApi *m_api = nullptr;
};
