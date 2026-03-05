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
    explicit SHPKPart(physis_SqPackResource *resource, QWidget *parent = nullptr);

    void load(physis_Buffer buffer);

private:
    void loadShader(const QModelIndex &index);
    void loadNode(const QModelIndex &index);
    void loadPass(const QModelIndex &index);
    void goToVertexShader(int index);
    void goToPixelShader(int index);
    void clearLayout(QLayout *layout);

    QTabWidget *pageTabWidget = nullptr;
    physis_SqPackResource *m_resource = nullptr;

    QWidget *shadersTab = nullptr;
    QListWidget *shadersListWidget = nullptr;
    QTextEdit *shadersTextEdit = nullptr;
    QListWidget *shadersScalarListWidget = nullptr;
    QListWidget *shadersTextureListWidget = nullptr;

    QTabWidget *keysTab = nullptr;
    QScrollArea *systemTab = nullptr;
    QVBoxLayout *systemLayout = nullptr;
    QScrollArea *sceneTab = nullptr;
    QVBoxLayout *sceneLayout = nullptr;
    QScrollArea *materialTab = nullptr;
    QVBoxLayout *materialLayout = nullptr;
    QScrollArea *subViewTab = nullptr;
    QVBoxLayout *subViewLayout = nullptr;

    QWidget *nodesTab = nullptr;
    QHBoxLayout *nodesLayout = nullptr;
    QListWidget *nodesListWidget = nullptr;
    QLineEdit *nodesSelectorEdit = nullptr;
    QTabWidget *nodesTabWidget = nullptr;
    QTabWidget *nodesKeysTabWidget = nullptr;

    QTabWidget *nodesKeysTab = nullptr;
    QScrollArea *nodesSystemTab = nullptr;
    QVBoxLayout *nodesSystemLayout = nullptr;
    QScrollArea *nodesSceneTab = nullptr;
    QVBoxLayout *nodesSceneLayout = nullptr;
    QScrollArea *nodesMaterialTab = nullptr;
    QVBoxLayout *nodesMaterialLayout = nullptr;
    QScrollArea *nodesSubViewTab = nullptr;
    QVBoxLayout *nodesSubViewLayout = nullptr;

    QWidget *nodesPassesTab = nullptr;
    QListWidget *nodesPassesListWidget = nullptr;
    QFormLayout *nodesPassesFormLayout = nullptr;
    QPushButton *vertexShaderButton = nullptr;
    QPushButton *pixelShaderButton = nullptr;

    QListWidget *scalarsListWidget;
    QListWidget *texturesListWidget;

    physis_SHPK m_shpk;

#ifdef HAVE_SYNTAX_HIGHLIGHTING
    KSyntaxHighlighting::Repository repository;
#endif
};
