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

class AbstractExcelResolver;

// TODO: rename to "EXDH" or "Excel" part or something similar because you cannot preview EXD on it's own
class EXDPart : public QWidget
{
    Q_OBJECT

public:
    explicit EXDPart(physis_SqPackResource *data, AbstractExcelResolver *resolver, QWidget *parent = nullptr);

    void loadSheet(const QString &name, physis_Buffer buffer);
    void goToRow(const QString &query);
    void resetSorting();

    void setPreferredLanguage(Language language);
    Language preferredLanguage() const;

    QList<QPair<QString, Language>> availableLanguages() const;
    QAction *selectLanguageAction() const;

private:
    void loadTables();

    physis_SqPackResource *data = nullptr;

    QTabWidget *pageTabWidget = nullptr;
    QFormLayout *headerFormLayout = nullptr;

    Language getSuitableLanguage(const physis_EXH &pExh) const;
    Language m_preferredLanguage = Language::English;
    physis_EXH exh;
    QString m_name;
    AbstractExcelResolver *m_resolver = nullptr;
    QAction *m_selectLanguage = nullptr;
    QMenu *m_languageMenu = nullptr;
    QActionGroup *m_languageGroup = nullptr;
};
