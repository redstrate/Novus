// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "luabpart.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QVBoxLayout>
#include <physis.hpp>

LuabPart::LuabPart(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    m_codeEdit = new QTextEdit();
    m_codeEdit->setReadOnly(true);
    layout->addWidget(m_codeEdit);

    setLayout(layout);
}

void LuabPart::load(physis_Buffer buffer)
{
    QTemporaryFile temporaryFile;
    if (temporaryFile.open()) {
        QFile file(temporaryFile.fileName());
        file.open(QIODevice::WriteOnly);
        file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
        file.close();

        QProcess luaDecProcess;
        luaDecProcess.setProgram(QStringLiteral("./luadec"));
        luaDecProcess.setArguments({temporaryFile.fileName()});
        luaDecProcess.start();
        luaDecProcess.waitForFinished();

        m_codeEdit->setText(QString::fromUtf8(luaDecProcess.readAllStandardOutput()));
    }
}

#include "moc_luabpart.cpp"