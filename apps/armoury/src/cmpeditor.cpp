// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmpeditor.h"
#include <KLocalizedString>

CmpEditor::CmpEditor(physis_SqPackResource *data, QWidget *parent)
    : CmpPart(data, parent)
{
    setWindowTitle(i18nc("@title:window CMP is an abbreviation", "CMP Editor"));

    load(physis_sqpack_read(data, "chara/xls/charamake/human.cmp"));
}

#include "moc_cmpeditor.cpp"
