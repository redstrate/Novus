// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtLogging>
#include <physis.hpp>

namespace utility
{
physis_Buffer readFromQrc(const QString &path);
}