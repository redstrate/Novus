// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
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

class QScrollArea;

class SHPKPart : public QWidget
{
    Q_OBJECT

public:
    explicit SHPKPart(QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer buffer);

private:
    void loadShader(const QModelIndex &index) const;
    void loadNode(const QModelIndex &index) const;
    void loadPass(const QModelIndex &index);
    void goToVertexShader(int index) const;
    void goToPixelShader(int index) const;
    static void clearLayout(QLayout *layout);

    QTabWidget *m_pageTabWidget = nullptr;

    QWidget *m_shadersTab = nullptr;
    QListWidget *m_shadersListWidget = nullptr;
    QTextEdit *m_shadersTextEdit = nullptr;
    QListWidget *m_shadersScalarListWidget = nullptr;
    QListWidget *m_shadersTextureListWidget = nullptr;

    QTabWidget *m_keysTab = nullptr;
    QScrollArea *m_systemTab = nullptr;
    QVBoxLayout *m_systemLayout = nullptr;
    QScrollArea *m_sceneTab = nullptr;
    QVBoxLayout *m_sceneLayout = nullptr;
    QScrollArea *m_materialTab = nullptr;
    QVBoxLayout *m_materialLayout = nullptr;
    QScrollArea *m_subViewTab = nullptr;
    QVBoxLayout *m_subViewLayout = nullptr;

    QWidget *m_nodesTab = nullptr;
    QHBoxLayout *m_nodesLayout = nullptr;
    QListWidget *m_nodesListWidget = nullptr;
    QLineEdit *m_nodesSelectorEdit = nullptr;
    QTabWidget *m_nodesTabWidget = nullptr;
    QTabWidget *m_nodesKeysTabWidget = nullptr;

    QTabWidget *m_nodesKeysTab = nullptr;
    QScrollArea *m_nodesSystemTab = nullptr;
    QVBoxLayout *m_nodesSystemLayout = nullptr;
    QScrollArea *m_nodesSceneTab = nullptr;
    QVBoxLayout *m_nodesSceneLayout = nullptr;
    QScrollArea *m_nodesMaterialTab = nullptr;
    QVBoxLayout *m_nodesMaterialLayout = nullptr;
    QScrollArea *m_nodesSubViewTab = nullptr;
    QVBoxLayout *m_nodesSubViewLayout = nullptr;

    QWidget *m_nodesPassesTab = nullptr;
    QListWidget *m_nodesPassesListWidget = nullptr;
    QFormLayout *m_nodesPassesFormLayout = nullptr;
    QPushButton *m_vertexShaderButton = nullptr;
    QPushButton *m_pixelShaderButton = nullptr;

    QListWidget *m_scalarsListWidget;
    QListWidget *m_texturesListWidget;

    physis_SHPK m_shpk{};

#ifdef HAVE_SYNTAX_HIGHLIGHTING
    KSyntaxHighlighting::Repository m_repository;
#endif
};
