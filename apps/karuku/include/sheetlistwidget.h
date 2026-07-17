// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QListView>
#include <QWidget>
#include <physis.hpp>

class SheetListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SheetListWidget(const physis_SqPackResource *data, QWidget *parent = nullptr);

    void focusSearchField() const;
    void goToSheet(const QString &name) const;

Q_SIGNALS:
    void sheetSelected(const QString &name);

private:
    QListView *m_listWidget = nullptr;

    QLineEdit *m_searchEdit = nullptr;
};
