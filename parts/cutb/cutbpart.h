// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWidget>
#include <physis.hpp>

class QListWidget;
class FileCache;
class ScenePart;
class CutbPart : public QWidget
{
    Q_OBJECT

public:
    explicit CutbPart(FileCache &cache, QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer file) const;

private:
    QHBoxLayout *m_layout = nullptr;
    ScenePart *m_part = nullptr;
    FileCache &m_cache;
    QListWidget *m_sceneListWidget = nullptr;
};
