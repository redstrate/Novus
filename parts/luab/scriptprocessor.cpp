// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scriptprocessor.h"

#include <QRegularExpression>

ScriptProcessor::ScriptProcessor()
{
    m_sceneFunction =
        ScriptFunction{.name = QStringLiteral("OnScene"), .scriptArguments = {QStringLiteral("self"), QStringLiteral("player"), QStringLiteral("actor")}};
    m_functions = {
        ScriptFunction{.name = QStringLiteral("IsTodoChecked"), .scriptArguments = {QStringLiteral("self"), QStringLiteral("player")}},
        ScriptFunction{.name = QStringLiteral("GetEventItems"), .scriptArguments = {QStringLiteral("self"), QStringLiteral("player")}},
        ScriptFunction{.name = QStringLiteral("OnInitialize"), .scriptArguments = {QStringLiteral("self")}},
        ScriptFunction{
            .name = QStringLiteral("IsAcceptEvent"),
            .scriptArguments = {QStringLiteral("self"), QStringLiteral("player")},
        },
        ScriptFunction{.name = QStringLiteral("GetTodoArgs"), .scriptArguments = {QStringLiteral("self"), QStringLiteral("player"), QStringLiteral("actor")}},
        ScriptFunction{.name = QStringLiteral("IsAnnounce"), .scriptArguments = {QStringLiteral("self"), QStringLiteral("player"), QStringLiteral("actor")}},
    };
}

QString ScriptProcessor::process(const QString &script)
{
    const QRegularExpression re(QStringLiteral(R"((\S+)\.(\S+) = \(function\(([^\n]+)\)(.*?)end\))"),
                                QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);

    QString parsedOutput = script;

    auto globalMatch = re.globalMatch(script);
    while (globalMatch.hasNext()) {
        const QRegularExpressionMatch match = globalMatch.next();

        const QString className = match.captured(1);
        const QString functionName = match.captured(2);
        const QStringList functionArgs = match.captured(3).remove(QLatin1Char(' ')).split(QLatin1Char(','));
        QString functionBody = match.captured(4);
        QString functionArgsList = match.captured(3);

        const auto replaceArguments = [&functionBody, &functionArgs, &functionArgsList](const QStringList &newArgs) {
            qsizetype i = 0;
            for (const auto &arg : functionArgs) {
                if (i < newArgs.size()) {
                    functionArgsList.replace(arg, newArgs[i]);
                    functionBody.replace(arg, newArgs[i]);
                }
                i++;
            }
        };

        if (functionName.startsWith(m_sceneFunction.name)) {
            replaceArguments(m_sceneFunction.scriptArguments);
        }
        for (const auto &function : m_functions) {
            if (functionName == function.name) {
                replaceArguments(function.scriptArguments);
            }
        }

        parsedOutput.replace(match.captured(4), functionBody);
        parsedOutput.replace(match.captured(3), functionArgsList);
    }

    return parsedOutput;
}
