// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMenuBar>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT OpenInWidget : public QMenuBar
{
    Q_OBJECT

public:
    OpenInWidget(QObject *target);
};
