// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>

struct ObjectScene;
struct physis_Sgb;
struct physis_Layer;
struct physis_InstanceObject;
class SceneState;

enum class TreeType {
    /// Root of the tree
    Root,
    /// LGB file
    File,
    /// A layer.
    Layer,
    /// A single object
    Object,
    /// A terrain plate.
    Plate,
};

struct TreeInformation {
    TreeType type;
    TreeInformation *parent = nullptr;
    int row = 0;
    QString name;
    uint32_t id;
    void const *data = nullptr;

    std::vector<TreeInformation *> children;
};

class SceneListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit SceneListModel(SceneState *appState, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    std::optional<physis_InstanceObject const *> objectAt(const QModelIndex &index) const;
    std::optional<physis_Layer const *> layerAt(const QModelIndex &index) const;

private:
    void refresh();
    void addLayer(uint32_t index, TreeInformation *fileItem, const physis_Layer &layer, ObjectScene &scene);
    void processScene(TreeInformation *parentNode, ObjectScene &scene);

    SceneState *m_appState = nullptr;
    TreeInformation *m_rootItem = nullptr;
};
