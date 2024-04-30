// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sklbpart.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_major_storage.hpp>

#include "quaternionedit.h"
#include "vec3edit.h"

void addItem(physis_Skeleton &skeleton, physis_Bone &bone, QTreeWidget *widget, QTreeWidgetItem *parent_item = nullptr)
{
    auto item = new QTreeWidgetItem();
    item->setText(0, QLatin1String(bone.name));

    if (parent_item == nullptr) {
        widget->addTopLevelItem(item);
    } else {
        parent_item->addChild(item);
    }

    for (uint32_t i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0)
            addItem(skeleton, skeleton.bones[i], widget, item);
    }
}

SklbPart::SklbPart(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout();
    setLayout(layout);

    boneListWidget = new QTreeWidget();
    boneListWidget->setHeaderLabel(i18nc("@title:column", "Name"));
    boneListWidget->setMaximumWidth(200);

    layout->addWidget(boneListWidget);

    transformLayout = new QVBoxLayout();
    layout->addLayout(transformLayout);

    auto transformGroup = new QGroupBox(i18nc("@title:group", "Bone Transform"));
    transformLayout->addWidget(transformGroup);
    auto transformGroupLayout = new QFormLayout();
    transformGroup->setLayout(transformGroupLayout);

    posEdit = new Vector3Edit(currentPosition);
    posEdit->setEnabled(false);
    connect(posEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->position, glm::value_ptr(currentPosition), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Position"), posEdit);

    rotationEdit = new QuaternionEdit(currentRotation);
    rotationEdit->setEnabled(false);
    connect(rotationEdit, &QuaternionEdit::onValueChanged, [this] {
        memcpy(currentEditedBone->rotation, glm::value_ptr(currentRotation), sizeof(float) * 4);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Rotation"), rotationEdit);

    scaleEdit = new Vector3Edit(currentScale);
    scaleEdit->setEnabled(false);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Scale"), scaleEdit);

    connect(boneListWidget, &QTreeWidget::itemClicked, this, &SklbPart::treeItemClicked);
}

void SklbPart::treeItemClicked(QTreeWidgetItem *item, int column)
{
    for (uint32_t i = 0; i < skeleton.num_bones; i++) {
        if (strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
            currentPosition = glm::make_vec3(skeleton.bones[i].position);
            currentRotation = glm::make_quat(skeleton.bones[i].rotation);
            currentScale = glm::make_vec3(skeleton.bones[i].scale);
            currentEditedBone = &skeleton.bones[i];

            posEdit->setEnabled(true);
            posEdit->setVector(currentPosition);

            rotationEdit->setEnabled(true);
            rotationEdit->setQuat(currentRotation);

            scaleEdit->setEnabled(true);
            scaleEdit->setVector(currentScale);

            if (racePosEdit != nullptr && raceRotationEdit != nullptr && raceScaleEdit != nullptr) {
                for (int j = 0; j < m_matrices.num_bones; j++) {
                    if (std::string_view{m_matrices.bones[j].name} == std::string_view{skeleton.bones[i].name}) {
                        auto deformBone = glm::rowMajor4(glm::vec4{m_matrices.bones[j].deform[0],
                                                                   m_matrices.bones[j].deform[1],
                                                                   m_matrices.bones[j].deform[2],
                                                                   m_matrices.bones[j].deform[3]},
                                                         glm::vec4{m_matrices.bones[j].deform[4],
                                                                   m_matrices.bones[j].deform[5],
                                                                   m_matrices.bones[j].deform[6],
                                                                   m_matrices.bones[j].deform[7]},
                                                         glm::vec4{m_matrices.bones[j].deform[8],
                                                                   m_matrices.bones[j].deform[9],
                                                                   m_matrices.bones[j].deform[10],
                                                                   m_matrices.bones[j].deform[11]},
                                                         glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
                        ;

                        glm::vec3 scale;
                        glm::quat rotation;
                        glm::vec3 translation;
                        glm::vec3 skew;
                        glm::vec4 perspective;
                        glm::decompose(deformBone, scale, rotation, translation, skew, perspective);

                        racePosEdit->setVector(translation);
                        raceRotationEdit->setQuat(rotation);
                        raceScaleEdit->setVector(scale);
                    }
                }
            }
        }
    }
}

void SklbPart::clear()
{
    boneListWidget->clear();
}

void SklbPart::load(physis_Skeleton file)
{
    clear();
    addItem(file, *file.root_bone, boneListWidget);
    skeleton = file;
}

void SklbPart::load_pbd(physis_PBD deformer, int from_body_id, int to_body_id)
{
    if (racePosEdit == nullptr && raceRotationEdit == nullptr && raceScaleEdit == nullptr) {
        auto raceTransformGroup = new QGroupBox(i18nc("@title:group", "Race Transform"));
        transformLayout->addWidget(raceTransformGroup);
        auto raceTransformGroupLayout = new QFormLayout();
        raceTransformGroup->setLayout(raceTransformGroupLayout);

        racePosEdit = new Vector3Edit(currentPosition);
        racePosEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Position"), racePosEdit);

        raceRotationEdit = new QuaternionEdit(currentRotation);
        raceRotationEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Rotation"), raceRotationEdit);

        raceScaleEdit = new Vector3Edit(currentScale);
        raceScaleEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Scale"), raceScaleEdit);
    }

    m_matrices = physis_pbd_get_deform_matrix(deformer, from_body_id, to_body_id);
}

#include "moc_sklbpart.cpp"