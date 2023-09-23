// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QMap>
#include <physis.hpp>

class EXDPart : public QWidget {
public:
    explicit EXDPart(GameData* data);

    void loadSheet(const QString& name);

private:
    GameData* data = nullptr;

    QTabWidget* pageTabWidget = nullptr;

    struct CachedExcel {
        physis_EXH* exh = nullptr;
        physis_EXD exd;
    };
    QMap<QString, CachedExcel> cachedExcelSheets;
    Language getSuitableLanguage(physis_EXH* pExh);
};