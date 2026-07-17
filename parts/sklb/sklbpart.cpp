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

void addItem(physis_Skeleton &skeleton, const physis_Bone &bone, QTreeWidget *widget, QTreeWidgetItem *parent_item = nullptr)
{
    const auto item = new QTreeWidgetItem();
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
    const auto layout = new QHBoxLayout();
    setLayout(layout);

    m_boneListWidget = new QTreeWidget();
    m_boneListWidget->setHeaderLabel(i18nc("@title:column", "Name"));
    m_boneListWidget->setMaximumWidth(200);

    layout->addWidget(m_boneListWidget);

    m_transformLayout = new QVBoxLayout();
    layout->addLayout(m_transformLayout);

    const auto transformGroup = new QGroupBox(i18nc("@title:group", "Bone Transform"));
    m_transformLayout->addWidget(transformGroup);
    const auto transformGroupLayout = new QFormLayout();
    transformGroup->setLayout(transformGroupLayout);

    m_posEdit = new Vector3Edit(m_currentPosition);
    m_posEdit->setEnabled(false);
    connect(m_posEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(m_currentEditedBone->position, glm::value_ptr(m_currentPosition), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Position"), m_posEdit);

    m_rotationEdit = new QuaternionEdit(m_currentRotation);
    m_rotationEdit->setEnabled(false);
    connect(m_rotationEdit, &QuaternionEdit::onValueChanged, [this] {
        memcpy(m_currentEditedBone->rotation, glm::value_ptr(m_currentRotation), sizeof(float) * 4);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Rotation"), m_rotationEdit);

    m_scaleEdit = new Vector3Edit(m_currentScale);
    m_scaleEdit->setEnabled(false);
    connect(m_scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(m_currentEditedBone->scale, glm::value_ptr(m_currentScale), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(i18nc("@label:spinbox", "Scale"), m_scaleEdit);

    connect(m_boneListWidget, &QTreeWidget::itemClicked, this, &SklbPart::treeItemClicked);
}

void SklbPart::treeItemClicked(const QTreeWidgetItem *item, const int column)
{
    for (uint32_t i = 0; i < m_skeleton.num_bones; i++) {
        if (strcmp(m_skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
            m_currentPosition = glm::make_vec3(m_skeleton.bones[i].position);
            m_currentRotation = glm::make_quat(m_skeleton.bones[i].rotation);
            m_currentScale = glm::make_vec3(m_skeleton.bones[i].scale);
            m_currentEditedBone = &m_skeleton.bones[i];

            QSignalBlocker posBlocker(m_posEdit);
            QSignalBlocker rotBlocker(m_rotationEdit);
            QSignalBlocker sclBlocker(m_scaleEdit);

            m_posEdit->setEnabled(true);
            m_posEdit->setVector(m_currentPosition);

            m_rotationEdit->setEnabled(true);
            m_rotationEdit->setQuat(m_currentRotation);

            m_scaleEdit->setEnabled(true);
            m_scaleEdit->setVector(m_currentScale);

            if (m_racePosEdit != nullptr && m_raceRotationEdit != nullptr && m_raceScaleEdit != nullptr) {
                for (int j = 0; j < m_matrices.num_bones; j++) {
                    if (std::string_view{m_matrices.bones[j].name} == std::string_view{m_skeleton.bones[i].name}) {
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

                        glm::vec3 scale;
                        glm::quat rotation;
                        glm::vec3 translation;
                        glm::vec3 skew;
                        glm::vec4 perspective;
                        glm::decompose(deformBone, scale, rotation, translation, skew, perspective);

                        m_racePosEdit->setVector(translation);
                        m_raceRotationEdit->setQuat(rotation);
                        m_raceScaleEdit->setVector(scale);
                    }
                }
            }
        }
    }
}

void SklbPart::clear() const
{
    m_boneListWidget->clear();
}

void SklbPart::load(physis_Skeleton file)
{
    clear();
    addItem(file, *file.root_bone, m_boneListWidget);
    m_skeleton = file;
}

void SklbPart::load_pbd(const physis_PBD deformer, const int from_body_id, const int to_body_id)
{
    if (m_racePosEdit == nullptr && m_raceRotationEdit == nullptr && m_raceScaleEdit == nullptr) {
        const auto raceTransformGroup = new QGroupBox(i18nc("@title:group", "Race Transform"));
        m_transformLayout->addWidget(raceTransformGroup);
        const auto raceTransformGroupLayout = new QFormLayout();
        raceTransformGroup->setLayout(raceTransformGroupLayout);

        m_racePosEdit = new Vector3Edit(m_currentPosition);
        m_racePosEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Position"), m_racePosEdit);

        m_raceRotationEdit = new QuaternionEdit(m_currentRotation);
        m_raceRotationEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Rotation"), m_raceRotationEdit);

        m_raceScaleEdit = new Vector3Edit(m_currentScale);
        m_raceScaleEdit->setEnabled(false);
        raceTransformGroupLayout->addRow(i18nc("@label:spinbox", "Scale"), m_raceScaleEdit);
    }

    m_matrices = physis_pbd_get_deform_matrix(deformer, from_body_id, to_body_id);
}

#include "moc_sklbpart.cpp"
