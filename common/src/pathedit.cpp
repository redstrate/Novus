// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathedit.h"

#include "launcherconfig.h"

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QProcess>

Q_GLOBAL_STATIC(OpenPathHandler, openPathHandler)

void OpenPathHandler::openPath(const QString &path)
{
    if (m_emitSignal) {
        Q_EMIT pathOpened(path);
    } else {
        QProcess::startDetached(DATAEXPLORER_EXECUTABLE, {path});
    }
}

void OpenPathHandler::setEmitSignal(const bool emit)
{
    m_emitSignal = emit;
}

PathEdit::PathEdit(QWidget *parent)
    : EditWidget(parent)
{
    const auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSizeConstraints(QLayout::SetMinAndMaxSize, QLayout::SetMinAndMaxSize);

    m_lineEdit = new QLineEdit();
    connect(m_lineEdit, &QLineEdit::editingFinished, this, &EditWidget::editingFinished);
    m_openAction = m_lineEdit->addAction(QIcon::fromTheme(QStringLiteral("document-open")), QLineEdit::TrailingPosition);
    m_openAction->setEnabled(false);
    connect(m_openAction, &QAction::triggered, this, [this] {
        openPathHandler->openPath(m_lineEdit->text());
    });
    layout->addWidget(m_lineEdit);
}

void PathEdit::setPath(const QString &path) const
{
    m_lineEdit->setText(path);
    m_openAction->setEnabled(!path.isEmpty());
}

void PathEdit::setReadOnly(const bool readOnly) const
{
    m_lineEdit->setReadOnly(readOnly);
}

OpenPathHandler *PathEdit::handler()
{
    return openPathHandler;
}

QString PathEdit::path() const
{
    return m_lineEdit->text();
}

#include "moc_pathedit.cpp"
