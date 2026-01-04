// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

class QComboBox;
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();

public Q_SLOTS:
    void configure();

private:
    void setupActions();
    void refreshGameInstalls();

    QComboBox *m_gameInstallCombo;
};
