// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

class ScriptProcessor
{
public:
    ScriptProcessor();

    QString process(const QString &script);

private:
    struct ScriptFunction {
        QString name;
        QStringList scriptArguments;
    };
    QList<ScriptFunction> m_functions;
    ScriptFunction m_sceneFunction;
};
