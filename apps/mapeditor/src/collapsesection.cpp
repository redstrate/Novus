// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collapsesection.h"

#include <QApplication>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

CollapseSection::CollapseSection(QString label, bool closable)
    : label(label)
    , closable(closable)
{
    setContentsMargins(0, 25, 0, 0);
    setMouseTracking(true);
}

void CollapseSection::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QColor(75, 75, 80));
    painter.setBrush(QColor(50, 50, 55));

    QRect r = event->rect().adjusted(1, 2, -1, 0);
    r.setY(2);
    r.setHeight(25);

    painter.drawRect(r);

    painter.setPen(Qt::white);
    painter.drawText(event->rect().adjusted(6, 5, 0, 0), label);

    if (closable) {
        QStyleOption option;
        option.rect.adjust(event->rect().width() - 20, 7, 0, 0);
        option.rect.setHeight(16);
        option.rect.setWidth(16);
        option.state = QStyle::State_Active | QStyle::State_Enabled | QStyle::State_AutoRaise;

        if (closeButtonHovered)
            option.state |= QStyle::State_Raised | QStyle::State_MouseOver;

        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &option, &painter, this);
    }
}

void CollapseSection::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (closable) {
        QRect r(width() - 20, 0, width(), 25); // close button

        if (r.contains(mapFromGlobal(QCursor::pos())))
            closeButtonHovered = true;
        else
            closeButtonHovered = false;

        repaint();
    }
}

void CollapseSection::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    QRect r(0, 0, width() - 20, 30); // header

    if (r.contains(mapFromGlobal(QCursor::pos()))) {
        if (!collapsed) {
            setFixedHeight(30);
            collapsed = true;
        } else {
            setFixedHeight(QWIDGETSIZE_MAX);
            collapsed = false;
        }
    }

    if (closable) {
        QRect r(width() - 20, 0, width(), 25); // close button

        if (r.contains(mapFromGlobal(QCursor::pos())))
            Q_EMIT closeRequested();
    }
}

#include "moc_collapsesection.cpp"
