// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"

#include <physis.hpp>

QString fromCString(const char *string)
{
    QString newString = QString::fromStdString(string);
    physis_free_string(string);
    return newString;
}

const char *toCString(const QString &value)
{
    std::string stdStringData = value.toStdString();
    char *cStringData = reinterpret_cast<char *>(malloc(stdStringData.length() + 1));
    strcpy(cStringData, stdStringData.data());

    return cStringData;
}
