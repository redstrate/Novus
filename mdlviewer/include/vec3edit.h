#pragma once

#include <QSpinBox>
#include <QWidget>
#include <glm/glm.hpp>

class Vector3Edit : public QWidget {
    Q_OBJECT
public:
    explicit Vector3Edit(glm::vec3& vec, QWidget* parent = nullptr);
    ~Vector3Edit();

    void setVector(glm::vec3& vec);

signals:
    void onValueChanged();

private:
    struct {
        QDoubleSpinBox *x, *y, *z;
    } spinBoxes;

    glm::vec3& vec;
    QTimer* updateTimer;
};
