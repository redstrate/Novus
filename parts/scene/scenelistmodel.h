// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>

struct DropInObject;
struct ScnSGActionControllerDescriptor;
struct physis_ScnTimeline;
class ObjectScene;
struct physis_Sgb;
struct physis_Layer;
struct physis_InstanceObject;
class SceneState;

enum class TreeType {
    /// Root of the tree
    Root,
    /// LGB file
    LgbFile,
    /// Embedded LGB file
    EmbeddedLgbFile,
    /// TERA file
    TeraFile,
    /// A layer.
    Layer,
    /// Representative of all timelines in this scene.
    Timelines,
    /// Representative of all action descriptors in this scene.
    Actions,
    /// A single object
    Object,
    /// A terrain plate.
    Plate,
    /// A motion timeline.
    Timeline,
    /// A action.
    Action,
    /// Drop-ins category.
    DropIns,
    /// Drop-in layer.
    DropInLayer,
    /// A drop-in object.
    DropInObject,
};

struct SceneTreeInformation {
    TreeType type = TreeType::Root;
    SceneTreeInformation *parent = nullptr;
    int row = 0;
    QString name;
    uint32_t id = 0;
    QVariant data;

    std::vector<SceneTreeInformation *> children{};
};

class SceneListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit SceneListModel(SceneState *appState, QObject *parent = nullptr);

    enum SceneListRoles {
        ObjectIdRole = Qt::UserRole,
    };
    Q_ENUM(SceneListRoles)

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    std::optional<physis_InstanceObject const *> objectAt(const QModelIndex &index) const;
    std::optional<physis_Layer const *> layerAt(const QModelIndex &index) const;
    std::optional<physis_ScnTimeline const *> timelineAt(const QModelIndex &index) const;
    std::optional<ScnSGActionControllerDescriptor const *> actionAt(const QModelIndex &index) const;
    std::optional<QString> lgbAt(const QModelIndex &index) const;
    std::optional<QString> teraAt(const QModelIndex &index) const;
    std::optional<DropInObject *> dropInObjectAt(const QModelIndex &index) const;

private:
    void refresh();
    void addLayer(uint32_t index, SceneTreeInformation *fileItem, const physis_Layer &layer, ObjectScene &scene);
    void processScene(SceneTreeInformation *parentNode, ObjectScene &scene);

    SceneState *m_appState = nullptr;
    SceneTreeInformation m_rootItem;
};
