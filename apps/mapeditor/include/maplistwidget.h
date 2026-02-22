// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QListView>
#include <physis.hpp>

class QSortFilterProxyModel;

class MapListWidget : public QDialog
{
    Q_OBJECT

public:
    explicit MapListWidget(physis_SqPackResource *data, QWidget *parent = nullptr);

    QString acceptedMap() const;
    int acceptedContentFinderCondition() const;

    void accept() override;

private:
    QListView *listWidget = nullptr;

    physis_SqPackResource *data = nullptr;
    QString m_acceptedMap;
    int m_acceptedContentFinderCondition = 0;
    QSortFilterProxyModel *m_searchModel = nullptr;
};
