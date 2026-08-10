// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "luabpart.h"

#include <KLocalizedString>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QVBoxLayout>
#include <physis.hpp>

#include "launcherconfig.h"
#include "scriptprocessor.h"
#include "texteditor.h"

LuabPart::LuabPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    m_codeEdit = new TextEditor();
    m_codeEdit->setHighlightingMode(QStringLiteral("glsl"));
    layout->addWidget(m_codeEdit);

    setLayout(layout);
}

void LuabPart::load(const physis_Buffer buffer) const
{
    QTemporaryFile temporaryFile;
    if (temporaryFile.open()) {
        QFile file(temporaryFile.fileName());
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
        file.close();

        QProcess luaDecProcess;
        luaDecProcess.setProgram(LUADEC_EXECUTABLE);
        luaDecProcess.setArguments({temporaryFile.fileName()});
        luaDecProcess.start();
        luaDecProcess.waitForFinished();

        ScriptProcessor processor;
        m_codeEdit->setText(processor.process(QString::fromUtf8(luaDecProcess.readAllStandardOutput())));
    }

    if (m_codeEdit->text().isEmpty()) {
        m_codeEdit->setText(i18n("Failed to decompile Lua bytecode, please report this as a bug!"));
    }
}

#include "moc_luabpart.cpp"
