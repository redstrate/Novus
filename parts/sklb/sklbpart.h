// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <physis.hpp>

#include "quaternionedit.h"
#include "vec3edit.h"

class SklbPart : public QWidget
{
    Q_OBJECT

public:
    explicit SklbPart(QWidget *parent = nullptr);

    void clear() const;
    void load(physis_Skeleton file);
    void load_pbd(physis_PBD deformer, int from_body_id, int to_body_id);

Q_SIGNALS:
    void valueChanged();

private:
    void treeItemClicked(const QTreeWidgetItem *item, int column);

    QTreeWidget *m_boneListWidget = nullptr;

    glm::vec3 m_currentPosition{};
    glm::quat m_currentRotation{};
    glm::vec3 m_currentScale{};

    physis_Bone *m_currentEditedBone = nullptr;

    Vector3Edit *m_posEdit = nullptr;
    QuaternionEdit *m_rotationEdit = nullptr;
    Vector3Edit *m_scaleEdit = nullptr;
    physis_Skeleton m_skeleton{};
    physis_PreBoneDeformMatrices m_matrices{};
    Vector3Edit *m_racePosEdit = nullptr;
    QuaternionEdit *m_raceRotationEdit = nullptr;
    Vector3Edit *m_raceScaleEdit = nullptr;

    QVBoxLayout *m_transformLayout = nullptr;
};
