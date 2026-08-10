// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

#include "novuscommon_export.h"

namespace KTextEditor
{
class Application;
class MainWindow;
class View;
class Document;
}
class QTextEdit;

/**
 * @brief Wrapper around KTextEditor which is currently optional on Windows.
 *
 * Once that limitation is removed this class will probably disappear.
 */
class NOVUSCOMMON_EXPORT TextEditor : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor() override = default;

    void setText(const QString &text) const;
    void setHighlightingMode(const QString &mode) const;
    void setBuffer(const QByteArray &array) const;

    QString text() const;

private:
    KTextEditor::Application *m_application = nullptr;
    KTextEditor::MainWindow *m_mainWindow = nullptr;
    KTextEditor::View *m_view = nullptr;
    KTextEditor::Document *m_document = nullptr;
    QTextEdit *m_textEdit = nullptr;
};
