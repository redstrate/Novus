// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QNetworkAccessManager>

class CachingExcelResolver;
class EXDPart;
struct SqPackResource;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SqPackResource *data);

private:
    void setupActions();

    SqPackResource *data = nullptr;
    QNetworkAccessManager *mgr = nullptr;
    EXDPart *m_exdPart = nullptr;
    CachingExcelResolver *m_excelResolver = nullptr;
};
