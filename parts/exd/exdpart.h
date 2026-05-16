// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <QComboBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QMap>
#include <QTabWidget>
#include <QWidget>
#include <physis.hpp>

class AbstractExcelResolver;

// TODO: rename to "EXDH" or "Excel" part or something similar because you cannot preview EXD on it's own
class EXDPart : public QWidget
{
    Q_OBJECT

public:
    explicit EXDPart(FileCache &cache, AbstractExcelResolver *resolver, QWidget *parent = nullptr);
    ~EXDPart() override;

    void loadSheet(const QString &name, physis_Buffer buffer);
    void goToRow(const QString &query);
    void resetSorting();
    void clear();
    void focusFilterField();
    void setReadOnly(bool readOnly);
    void save();

    void setPreferredLanguage(Language language);
    Language preferredLanguage() const;

    QList<QPair<QString, Language>> availableLanguages() const;
    QAction *selectLanguageAction() const;
    QAction *saveCsvAction();
    QString name() const;

    QString selectedRow() const;
    bool isModified() const;

    struct SearchSettings {
        int column = -1;
        bool caseSensitive = false;
        bool enableRegex = false;
    };

Q_SIGNALS:
    void requestJump(const QString &name, const QString &rowQuery);
    void modified();

private:
    void loadTables();
    void filterData(const QString &pattern);
    void setSearchSettings(SearchSettings newSettings);

    FileCache &m_cache;

    QTabWidget *m_pageTabWidget = nullptr;
    QFormLayout *m_headerFormLayout = nullptr;

    Language getSuitableLanguage(const physis_EXH &pExh) const;
    Language m_preferredLanguage;
    physis_EXH m_exh{};
    QString m_name;
    AbstractExcelResolver *m_resolver = nullptr;
    QAction *m_selectLanguage = nullptr;
    QMenu *m_languageMenu = nullptr;
    QActionGroup *m_languageGroup = nullptr;
    QAction *m_saveCsvAction = nullptr;
    physis_ExcelSheet m_sheet{};
    QLineEdit *m_filterEdit = nullptr;
    bool m_modified = false;
    bool m_readOnly = false;

    SearchSettings m_searchSettings;
};
