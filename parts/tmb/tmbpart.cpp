// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tmbpart.h"

#include <QVBoxLayout>
#include <physis.hpp>

TmbPart::TmbPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    setLayout(layout);
}

void TmbPart::load(const Platform platform, const physis_Buffer buffer)
{
    const auto tmb = physis_tmb_parse(platform, buffer);
    loadExisting(tmb);
}

void TmbPart::loadExisting(const physis_Tmb tmb)
{
    m_tmb = tmb;
}

#include "moc_tmbpart.cpp"
