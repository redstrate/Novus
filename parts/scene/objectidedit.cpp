// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectidedit.h"

#include "scenestate.h"

#include <QHBoxLayout>

ObjectIdEdit::ObjectIdEdit(SceneState *state, QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    setMaximumHeight(35); // FIXME: don't hard-code
    layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    layout->addWidget(m_lineEdit);

    m_goToButton = new QPushButton();
    m_goToButton->setEnabled(false);
    m_goToButton->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(m_goToButton, &QPushButton::clicked, this, [this, state] {
        Q_EMIT state->selectObject(m_objectId);
    });
    layout->addWidget(m_goToButton);
}

void ObjectIdEdit::setObjectId(const uint32_t objectId)
{
    m_objectId = objectId;
    m_lineEdit->setText(QString::number(objectId));
    m_goToButton->setEnabled(m_objectId != 0);
}

#include "moc_objectidedit.cpp"
