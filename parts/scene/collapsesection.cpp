// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collapsesection.h"

#include <QApplication>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

CollapseSection::CollapseSection(const QString &label)
    : m_label(label)
{
    setContentsMargins(0, 30, 0, 0);
}

void CollapseSection::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(palette().dark().color());
    painter.setBrush(palette().alternateBase().color());
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();
    r.setHeight(25);

    painter.drawRoundedRect(r, 5, 5);

    painter.setPen(palette().text().color());
    painter.drawText(rect().adjusted(6, 4, 0, 0), m_label);

    QStyleOption option;
    option.rect.adjust(rect().width() - 20, 5, 0, 0);
    option.rect.setHeight(16);
    option.rect.setWidth(16);
    option.state = QStyle::State_Active | QStyle::State_Enabled | QStyle::State_AutoRaise;

    QApplication::style()->drawPrimitive(m_collapsed ? QStyle::PE_IndicatorArrowUp : QStyle::PE_IndicatorArrowDown, &option, &painter, this);
}

void CollapseSection::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    const QRect r(0, 0, width(), 30); // header

    if (r.contains(mapFromGlobal(QCursor::pos()))) {
        if (!m_collapsed) {
            setFixedHeight(25);
            m_collapsed = true;
        } else {
            setFixedHeight(QWIDGETSIZE_MAX);
            m_collapsed = false;
        }
    }
}

#include "moc_collapsesection.cpp"
