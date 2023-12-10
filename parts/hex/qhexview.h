// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#pragma once

#include "document/qhexdocument.h"
#include "document/qhexrenderer.h"
#include <QAbstractScrollArea>
#include <QTimer>

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit QHexView(QWidget *parent = nullptr);
    QHexDocument *document();
    void setDocument(QHexDocument *document);
    void setReadOnly(bool b);

protected:
    bool event(QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private Q_SLOTS:
    void renderCurrentLine();
    void moveToSelection();
    void blinkCursor();

private:
    void moveNext(bool select = false);
    void movePrevious(bool select = false);

private:
    bool processMove(QHexCursor *cur, QKeyEvent *e);
    bool processTextInput(QHexCursor *cur, QKeyEvent *e);
    bool processAction(QHexCursor *cur, QKeyEvent *e);
    void adjustScrollBars();
    void renderLine(quint64 line);
    quint64 firstVisibleLine() const;
    quint64 lastVisibleLine() const;
    quint64 visibleLines() const;
    bool isLineVisible(quint64 line) const;

    int documentSizeFactor() const;

    QPoint absolutePosition(const QPoint &pos) const;

private:
    QHexDocument *m_document;
    QHexRenderer *m_renderer;
    QTimer *m_blinktimer;
    bool m_readonly;
};
