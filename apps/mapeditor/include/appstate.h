// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

#include <physis.hpp>

class AppState : public QObject
{
    Q_OBJECT

public:
    explicit AppState(QObject *parent = nullptr);

    physis_LayerGroup bgGroup;

Q_SIGNALS:
    void mapLoaded();
};
