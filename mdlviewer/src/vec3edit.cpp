#include "vec3edit.h"

#include <QHBoxLayout>
#include <QTimer>

Vector3Edit::Vector3Edit(glm::vec3& vec, QWidget* parent) : QWidget(parent), vec(vec) {
    QHBoxLayout* itemsLayout = new QHBoxLayout(this);

    spinBoxes.x = new QDoubleSpinBox();
    spinBoxes.y = new QDoubleSpinBox();
    spinBoxes.z = new QDoubleSpinBox();

    spinBoxes.x->setMinimum(-10000.0);
    spinBoxes.x->setMaximum(10000.0);

    spinBoxes.y->setMinimum(-10000.0);
    spinBoxes.y->setMaximum(10000.0);

    spinBoxes.z->setMinimum(-10000.0);
    spinBoxes.z->setMaximum(10000.0);

    itemsLayout->addWidget(spinBoxes.x);
    itemsLayout->addWidget(spinBoxes.y);
    itemsLayout->addWidget(spinBoxes.z);

    spinBoxes.x->setValue(vec.x);
    spinBoxes.y->setValue(vec.y);
    spinBoxes.z->setValue(vec.z);

    connect(spinBoxes.x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.x = d;
        emit onValueChanged();
    });
    connect(spinBoxes.y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.y = d;
        emit onValueChanged();
    });
    connect(spinBoxes.z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.z = d;
        emit onValueChanged();
    });

    // TODO: find a better way to do this
    updateTimer = new QTimer();
    connect(updateTimer, &QTimer::timeout, [this, &vec]() {
        if (vec.x != spinBoxes.x->value() || vec.y != spinBoxes.y->value() || vec.z != spinBoxes.z->value()) {
            spinBoxes.x->setValue(vec.x);
            spinBoxes.y->setValue(vec.y);
            spinBoxes.z->setValue(vec.z);
        }
    });

    updateTimer->start(1);
}

Vector3Edit::~Vector3Edit() {
    updateTimer->stop();
}
