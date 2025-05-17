// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

#include <physis.hpp>

class AppState : public QObject
{
    Q_OBJECT

public:
    explicit AppState(QObject *parent = nullptr);

    QString basePath;
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    QList<uint32_t> visibleLayerIds;

Q_SIGNALS:
    void mapLoaded();
    void visibleLayerIdsChanged();
};
