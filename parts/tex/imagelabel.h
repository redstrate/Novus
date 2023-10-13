// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLabel>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = nullptr);

    [[nodiscard]] int heightForWidth(int width) const override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QPixmap scaledPixmap() const;

public Q_SLOTS:
    void setQPixmap(const QPixmap &);
    void resizeEvent(QResizeEvent *) override;

private:
    QPixmap pix;
};
