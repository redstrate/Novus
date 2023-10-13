// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMdiSubWindow>
#include <physis.hpp>

#include "filetreemodel.h"

class FileTreeWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FileTreeWindow(QString gamePath, GameData *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void extractFile(QString path);
    void pathSelected(QString path);

private:
    GameData *data = nullptr;
    FileTreeModel *m_fileModel;
};