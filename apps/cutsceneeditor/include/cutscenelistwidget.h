// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QListView>
#include <physis.hpp>

class FileCache;
class QSortFilterProxyModel;

class CutsceneListWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CutsceneListWidget(FileCache &cache, QWidget *parent = nullptr);

    QString acceptedCutscene() const;

    void accept() override;

private:
    QListView *m_listWidget = nullptr;

    FileCache &m_cache;
    QString m_acceptedCutscene;
    QSortFilterProxyModel *m_searchModel = nullptr;
};
