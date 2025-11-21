// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
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

class SHPKPart : public QWidget
{
    Q_OBJECT

public:
    explicit SHPKPart(QWidget *parent = nullptr);

    void load(physis_Buffer buffer);

private:
    void loadShader(const QModelIndex &index);
    void loadNode(const QModelIndex &index);
    void loadPass(const QModelIndex &index);
    void goToVertexShader(int index);
    void goToPixelShader(int index);
    void clearLayout(QLayout *layout);

    QTabWidget *pageTabWidget = nullptr;

    QWidget *shadersTab = nullptr;
    QListWidget *shadersListWidget = nullptr;
    QTextEdit *shadersTextEdit = nullptr;
    QListWidget *shadersScalarListWidget = nullptr;
    QListWidget *shadersTextureListWidget = nullptr;

    QTabWidget *keysTab = nullptr;
    QWidget *systemTab = nullptr;
    QVBoxLayout *systemLayout = nullptr;
    QWidget *sceneTab = nullptr;
    QVBoxLayout *sceneLayout = nullptr;
    QWidget *materialTab = nullptr;
    QVBoxLayout *materialLayout = nullptr;
    QWidget *subViewTab = nullptr;
    QVBoxLayout *subViewLayout = nullptr;

    QWidget *nodesTab = nullptr;
    QHBoxLayout *nodesLayout = nullptr;
    QListWidget *nodesListWidget = nullptr;
    QLineEdit *nodesSelectorEdit = nullptr;
    QTabWidget *nodesTabWidget = nullptr;
    QTabWidget *nodesKeysTabWidget = nullptr;

    QTabWidget *nodesKeysTab = nullptr;
    QWidget *nodesSystemTab = nullptr;
    QVBoxLayout *nodesSystemLayout = nullptr;
    QWidget *nodesSceneTab = nullptr;
    QVBoxLayout *nodesSceneLayout = nullptr;
    QWidget *nodesMaterialTab = nullptr;
    QVBoxLayout *nodesMaterialLayout = nullptr;
    QWidget *nodesSubViewTab = nullptr;
    QVBoxLayout *nodesSubViewLayout = nullptr;

    QWidget *nodesPassesTab = nullptr;
    QListWidget *nodesPassesListWidget = nullptr;
    QFormLayout *nodesPassesFormLayout = nullptr;
    QPushButton *vertexShaderButton = nullptr;
    QPushButton *pixelShaderButton = nullptr;

    QListWidget *scalarsListWidget;
    QListWidget *texturesListWidget;

    physis_SHPK m_shpk;

    KSyntaxHighlighting::Repository repository;
};
