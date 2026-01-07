// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class CollapseSection : public QWidget
{
    Q_OBJECT

public:
    explicit CollapseSection(QString label, bool closable = false);

Q_SIGNALS:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString label;

    bool closable;
    bool closeButtonHovered = false;
    bool collapsed = false;

    QWidget *wrapper;
    QLayout *layout;
};
