// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KSyntaxHighlighting/Repository>
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
    explicit SHCDPart(QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer buffer);

private:
    physis_SHCD m_shcd;
    QTextEdit *m_shaderTextEdit = nullptr;

    KSyntaxHighlighting::Repository repository;
};
