// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tmbpart.h"

#include <QVBoxLayout>
#include <physis.hpp>

TmbPart::TmbPart(physis_SqPackResource *resource, QWidget *parent)
    : QWidget(parent)
    , m_resource(resource)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);
}

void TmbPart::load(physis_Buffer buffer)
{
    auto tmb = physis_tmb_parse(m_resource->platform, buffer);
    loadExisting(tmb);
}

void TmbPart::loadExisting(physis_Tmb tmb)
{
    m_tmb = tmb;
}

#include "moc_tmbpart.cpp"
