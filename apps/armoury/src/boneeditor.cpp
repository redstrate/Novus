// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "boneeditor.h"

#include "gearview.h"

BoneEditor::BoneEditor(GearView *gearView, QWidget *parent)
    : SklbPart(parent)
{
    connect(&gearView->part(), &MDLPart::skeletonChanged, this, [this, gearView] {
        load(*gearView->part().skeleton);
    });

    connect(this, &SklbPart::valueChanged, &gearView->part(), &MDLPart::reloadRenderer);

    if (gearView->part().skeleton) {
        load(*gearView->part().skeleton);
    }
}

#include "moc_boneeditor.cpp"