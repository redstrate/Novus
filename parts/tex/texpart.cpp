// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texpart.h"

#include <QVBoxLayout>
#include <physis.hpp>

TexPart::TexPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    m_label = new ImageLabel();
    layout->addWidget(m_label);
}

void TexPart::load(physis_Buffer file)
{
    auto tex = physis_texture_parse(file);

    QImage image(tex.rgba, tex.width, tex.height, QImage::Format_RGBA8888);
    m_label->setQPixmap(QPixmap::fromImage(image));
}

#include "moc_texpart.cpp"