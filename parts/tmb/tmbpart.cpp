// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tmbpart.h"

#include <QVBoxLayout>
#include <physis.hpp>

TmbPart::TmbPart(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);
}

void TmbPart::load(physis_Buffer buffer)
{
}

#include "moc_tmbpart.cpp"
