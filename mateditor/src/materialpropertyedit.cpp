// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "materialpropertyedit.h"

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QPushButton>

MaterialPropertyEdit::MaterialPropertyEdit(QWidget *parent)
    : QWidget(parent)
{
    m_itemsLayout = new QVBoxLayout(this);

    auto shaderPackageLayout = new QHBoxLayout();
    m_itemsLayout->addLayout(shaderPackageLayout);

    m_shaderPackageName = new QLineEdit();
    m_shaderPackageName->setReadOnly(true);
    shaderPackageLayout->addWidget(m_shaderPackageName);

    auto selectShaderPackageButton = new QPushButton(i18n("Shaders…"));
    shaderPackageLayout->addWidget(selectShaderPackageButton);

    setLayout(m_itemsLayout);

    rebuild();
}

void MaterialPropertyEdit::setMaterial(physis_Material material)
{
    m_shaderPackageName->setText(QString::fromLatin1(material.shpk_name));
}

void MaterialPropertyEdit::rebuild()
{
}

#include "moc_materialpropertyedit.cpp"
