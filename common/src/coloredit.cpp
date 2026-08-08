// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coloredit.h"

#include <QHBoxLayout>
#include <QPushButton>

static QColor fromVec3(const Color vec)
{
    return QColor::fromRgb(vec.red, vec.green, vec.blue, vec.alpha);
}

static Color fromQColor(const QColor color)
{
    Color vec{};

    int r, g, b, a;
    color.getRgb(&r, &g, &b, &a);

    vec.red = r;
    vec.green = g;
    vec.blue = b;
    vec.alpha = a;

    return vec;
}

ColorEdit::ColorEdit(QWidget *parent)
    : EditWidget(parent)
{
    const auto itemsLayout = new QHBoxLayout(this);

    colorButton = new QPushButton();
    colorButton->setFlat(true);

    colorButton->setAutoFillBackground(true);

    connect(colorButton, &QPushButton::clicked, [this](bool) {
        if (reference != nullptr) {
            const QColor oldcolor = fromVec3(*reference);
            const QColor newcolor = QColorDialog::getColor(oldcolor);

            *reference = fromQColor(newcolor);

            rebuild();

            Q_EMIT onValueChanged();
            Q_EMIT editingFinished();
        }
    });

    itemsLayout->addWidget(colorButton);
}

void ColorEdit::rebuild() const
{
    QPalette pal = colorButton->palette();
    pal.setColor(QPalette::Button, fromVec3(*reference));
    colorButton->setPalette(pal);
    colorButton->update();
}

#include "moc_coloredit.cpp"
