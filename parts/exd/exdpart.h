// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFormLayout>
#include <QMap>
#include <QTabWidget>
#include <QWidget>
#include <physis.hpp>

// TODO: rename to "EXDH" or "Excel" part or something similar because you cannot preview EXD on it's own
class EXDPart : public QWidget {
public:
    explicit EXDPart(GameData* data);

    void loadSheet(QString name, physis_Buffer buffer);

private:
    GameData* data = nullptr;

    QTabWidget* pageTabWidget = nullptr;
    QFormLayout *headerFormLayout = nullptr;

    struct CachedExcel {
        physis_EXH* exh = nullptr;
        physis_EXD exd;
    };
    QMap<QString, CachedExcel> cachedExcelSheets;
    Language getSuitableLanguage(physis_EXH* pExh);
};