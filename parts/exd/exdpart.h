// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QComboBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QMap>
#include <QTabWidget>
#include <QWidget>
#include <physis.hpp>

// TODO: rename to "EXDH" or "Excel" part or something similar because you cannot preview EXD on it's own
class EXDPart : public QWidget
{
    Q_OBJECT

public:
    explicit EXDPart(SqPackResource *data, QWidget *parent = nullptr);

    void loadSheet(const QString &name, physis_Buffer buffer);
    void goToRow(const QString &query);

private:
    void loadTables();

    SqPackResource *data = nullptr;

    QTabWidget *pageTabWidget = nullptr;
    QFormLayout *headerFormLayout = nullptr;
    QComboBox *languageComboBox = nullptr;

    struct CachedExcel {
        physis_EXH *exh = nullptr;
        physis_EXD exd{};
    };
    QMap<QString, CachedExcel> cachedExcelSheets;
    Language getSuitableLanguage(physis_EXH *pExh);
    Language preferredLanguage = Language::English;
    physis_EXH *exh = nullptr;
    QString name;
};
