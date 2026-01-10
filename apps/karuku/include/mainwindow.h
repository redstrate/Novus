// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QActionGroup>
#include <QNetworkAccessManager>
#include <physis.hpp>

class SheetListWidget;
class CachingExcelResolver;
class EXDPart;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);

private:
    void setupActions();

    physis_SqPackResource m_data;
    QNetworkAccessManager *mgr = nullptr;
    EXDPart *m_exdPart = nullptr;
    CachingExcelResolver *m_excelResolver = nullptr;
    SheetListWidget *m_sheetListWidget = nullptr;
};
