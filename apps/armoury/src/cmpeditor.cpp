// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmpeditor.h"

#include "filecache.h"

#include <KLocalizedString>

CmpEditor::CmpEditor(FileCache &cache, QWidget *parent)
    : CmpPart(parent)
{
    setWindowTitle(i18nc("@title:window CMP is an abbreviation", "CMP Editor"));

    load(cache.platform(), cache.lookupFile(QStringLiteral("chara/xls/charamake/human.cmp")));
}

#include "moc_cmpeditor.cpp"
