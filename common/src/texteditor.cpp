// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditor.h"

#include <QTemporaryFile>
#include <QVBoxLayout>

#ifdef HAVE_TEXT_EDITOR
#include <KTextEditor/Application>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>
#include <KTextEditor/View>
#else
#include <QTextEdit>
#endif

TextEditor::TextEditor(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins({0, 0, 0, 0});
    layout->setSpacing(0);
    setLayout(layout);

#ifdef HAVE_TEXT_EDITOR
    m_application = new KTextEditor::Application(this);
    m_mainWindow = new KTextEditor::MainWindow(this);

    const auto editor = KTextEditor::Editor::instance();
    KTextEditor::Editor::instance()->setApplication(m_application);

    m_document = editor->createDocument(this);

    m_view = m_document->createView(this);
    m_view->setConfigValue(QStringLiteral("modification-markers"), false);
    m_view->setConfigValue(QStringLiteral("icon-bar"), false);
    m_view->setConfigValue(QStringLiteral("allow-mark-menu"), false);
    layout->addWidget(m_view);
#else
    m_textEdit = new QTextEdit();
    m_textEdit->setFontFamily(QStringLiteral("monospace"));
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit);
#endif
}

void TextEditor::setText(const QString &text) const
{
#ifdef HAVE_TEXT_EDITOR
    m_document->setReadWrite(true);
    m_document->setText(text);
    m_document->setReadWrite(false);
    m_view->setCursorPosition(KTextEditor::Cursor(0, 0)); // move back to the beginning of the document
#else
    m_textEdit->setPlainText(text);
#endif
}

void TextEditor::setHighlightingMode(const QString &mode) const
{
#ifdef HAVE_TEXT_EDITOR
    m_document->setHighlightingMode(mode);
#else
    Q_UNUSED(mode)
#endif
}

void TextEditor::setBuffer(const QByteArray &array) const
{
#ifdef HAVE_TEXT_EDITOR
    QTemporaryFile file;
    Q_UNUSED(file.open())
    file.write(array);
    file.flush();

    m_document->setReadWrite(true);
    m_document->openUrl(QUrl::fromLocalFile(file.fileName()));
    m_document->setReadWrite(false);
    m_view->setCursorPosition(KTextEditor::Cursor(0, 0)); // move back to the beginning of the document
#else
    m_textEdit->setText(QString::fromUtf8(array));
#endif
}

QString TextEditor::text() const
{
#ifdef HAVE_TEXT_EDITOR
    return m_document->text();
#else
    return m_textEdit->toPlainText();
#endif
}

#include "moc_texteditor.cpp"
