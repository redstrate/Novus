// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>

class NovusMainWindow : public QMainWindow
{
public:
    explicit NovusMainWindow();

protected:
    void setupMenubar();

    virtual void setupFileMenu(QMenu *menu)
    {
        Q_UNUSED(menu)
    }

    virtual void setupAdditionalMenus(QMenuBar *menuBar)
    {
        Q_UNUSED(menuBar)
    }
};