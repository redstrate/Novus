// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent)
    : QLabel(parent)
{
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}

void ImageLabel::setQPixmap(const QPixmap &p)
{
    m_pix = p;
    setPixmap(scaledPixmap());
}

int ImageLabel::heightForWidth(const int width) const
{
    return m_pix.isNull() ? height() : m_pix.height() * width / m_pix.width();
}

QSize ImageLabel::sizeHint() const
{
    const int w = this->width();
    return {w, heightForWidth(w)};
}

QPixmap ImageLabel::scaledPixmap() const
{
    return m_pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ImageLabel::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    if (!m_pix.isNull()) {
        setPixmap(scaledPixmap());
    }
}

#include "moc_imagelabel.cpp"
