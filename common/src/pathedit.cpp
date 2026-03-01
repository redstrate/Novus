// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathedit.h"

#include "launcherconfig.h"

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
    auto layout = new QHBoxLayout(this);
    setMaximumHeight(35); // FIXME: don't hard-code
    layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    layout->addWidget(m_lineEdit);

    m_openButton = new QPushButton();
    m_openButton->setEnabled(false);
    m_openButton->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(m_openButton, &QPushButton::clicked, this, [this] {
        openPathHandler->openPath(m_lineEdit->text());
    });
    layout->addWidget(m_openButton);
}

void PathEdit::setPath(const QString &path)
{
    m_lineEdit->setText(path);
    m_openButton->setEnabled(!path.isEmpty());
}

void PathEdit::setReadOnly(const bool readOnly)
{
    m_lineEdit->setReadOnly(readOnly);
}

OpenPathHandler *PathEdit::handler()
{
    return openPathHandler;
}

#include "moc_pathedit.cpp"
