#pragma once

#include <QSpinBox>
#include <QWidget>
#include <glm/glm.hpp>
#include <physis.hpp>
#include <glm/detail/type_quat.hpp>

class GearView;

class BoneEditor : public QWidget {
    Q_OBJECT

public:
    explicit BoneEditor(GearView* gearView, QWidget* parent = nullptr);

private:
    glm::vec3 currentPosition;
    glm::quat currentRotation;
    glm::vec3 currentScale;
    physis_Bone* currentEditedBone;
};
