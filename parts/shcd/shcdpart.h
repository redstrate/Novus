// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef HAVE_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/Repository>
#endif
#include <QFormLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <physis.hpp>

class SHCDPart : public QWidget
{
    Q_OBJECT

public:
    explicit SHCDPart(physis_SqPackResource *resource, QWidget *parent = nullptr);

    void load(physis_Buffer buffer);

private:
    physis_SqPackResource *m_resource = nullptr;
    physis_SHCD m_shcd;
    QTextEdit *m_shaderTextEdit = nullptr;

#ifdef HAVE_SYNTAX_HIGHLIGHTING
    KSyntaxHighlighting::Repository repository;
#endif
};
