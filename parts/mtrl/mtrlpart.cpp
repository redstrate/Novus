// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mtrlpart.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <physis.hpp>

#include "knownvalues.h"
#include "texpart.h"

MtrlPart::MtrlPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , m_data(data)
{
    m_itemsLayout = new QVBoxLayout(this);

    auto shaderPackageLayout = new QHBoxLayout();
    m_itemsLayout->addLayout(shaderPackageLayout);

    m_shaderPackageName = new QLineEdit();
    m_shaderPackageName->setReadOnly(true);
    shaderPackageLayout->addWidget(m_shaderPackageName);

    auto selectShaderPackageButton = new QPushButton(i18n("Shaders…"));
    shaderPackageLayout->addWidget(selectShaderPackageButton);

    m_tabWidget = new QTabWidget();
    m_itemsLayout->addWidget(m_tabWidget);

    auto propertiesTab = new QWidget();
    m_propertiesLayout = new QVBoxLayout();
    propertiesTab->setLayout(m_propertiesLayout);

    auto texturesTab = new QWidget();
    m_texturesLayout = new QVBoxLayout();
    texturesTab->setLayout(m_texturesLayout);

    auto constantsTab = new QWidget();
    m_constantsLayout = new QVBoxLayout();
    constantsTab->setLayout(m_constantsLayout);

    m_tabWidget->addTab(propertiesTab, i18n("Parameters"));
    m_tabWidget->addTab(texturesTab, i18n("Textures"));
    m_tabWidget->addTab(constantsTab, i18n("Constants"));

    setLayout(m_itemsLayout);

    rebuild();
}

void MtrlPart::load(physis_Material file)
{
    m_material = file;
    m_shaderPackageName->setText(QString::fromLatin1(m_material.shpk_name));
    if (m_material.shpk_name != nullptr) {
        std::string shpkPath = "shader/sm5/shpk/" + std::string(m_material.shpk_name);

        auto shpkData = physis_gamedata_extract_file(m_data, shpkPath.c_str());
        if (shpkData.data != nullptr) {
            m_shpk = physis_parse_shpk(shpkData);
        }
    }
    rebuild();
}

void MtrlPart::rebuild()
{
    QLayoutItem *child = nullptr;
    while ((child = m_propertiesLayout->takeAt(0)) != nullptr) {
        child->widget()->setParent(nullptr);
        child->widget()->deleteLater();
    }

    for (int i = 0; i < m_shpk.num_material_keys; i++) {
        const auto materialKey = m_shpk.material_keys[i];

        auto groupBox = new QGroupBox();
        m_propertiesLayout->addWidget(groupBox);

        if (keys.contains(materialKey.id)) {
            groupBox->setTitle(QString::fromLatin1(keys[materialKey.id]));
        } else {
            groupBox->setTitle(i18n("Unknown Property %1", QStringLiteral("%1").arg(materialKey.id, 1, 16)));
        }

        uint32_t value = 0;

        bool found = false;
        for (int j = 0; j < m_material.num_shader_keys; j++) {
            auto shaderKey = m_material.shader_keys[j];

            if (shaderKey.category == materialKey.id) {
                value = shaderKey.value;
                found = true;
            }
        }

        // Fall back to default value
        if (!found) {
            value = materialKey.default_value;
        }

        auto layout = new QFormLayout();
        groupBox->setLayout(layout);

        auto label = new QLabel();
        if (keys.contains(value)) {
            label->setText(QString::fromLatin1(keys[value]));
        } else {
            label->setText(i18n("Unknown value %1", QStringLiteral("%1").arg(value, 1, 16)));
        }

        layout->addRow(i18n("Value:"), label);
    }

    child = nullptr;
    while ((child = m_texturesLayout->takeAt(0)) != nullptr) {
        child->widget()->setParent(nullptr);
        child->widget()->deleteLater();
    }

    for (int i = 0; i < m_material.num_samplers; i++) {
        const auto sampler = m_material.samplers[i];

        QString name;
        switch (sampler.texture_usage) {
        case TextureUsage::Sampler:
        case TextureUsage::Sampler0:
        case TextureUsage::Sampler1:
            name = i18n("Generic");
            break;
        case TextureUsage::SamplerCatchlight:
            name = i18n("Catchlight");
            break;
        case TextureUsage::SamplerColorMap0:
            name = i18n("Color Map 0");
            break;
        case TextureUsage::SamplerColorMap1:
            name = i18n("Color Map 1");
            break;
        case TextureUsage::SamplerDiffuse:
            name = i18n("Diffuse");
            break;
        case TextureUsage::SamplerEnvMap:
            name = i18n("Environment Map");
            break;
        case TextureUsage::SamplerMask:
            name = i18n("Mask");
            break;
        case TextureUsage::SamplerNormal:
            name = i18n("Normal");
            break;
        case TextureUsage::SamplerNormalMap0:
            name = i18n("Normal Map 0");
            break;
        case TextureUsage::SamplerNormalMap1:
            name = i18n("Normal Map 1");
            break;
        case TextureUsage::SamplerReflection:
            name = i18n("Reflection");
            break;
        case TextureUsage::SamplerSpecular:
            name = i18n("Specular");
            break;
        case TextureUsage::SamplerSpecularMap0:
            name = i18n("Specular Map 0");
            break;
        case TextureUsage::SamplerSpecularMap1:
            name = i18n("Specular Map 1");
            break;
        case TextureUsage::SamplerWaveMap:
            name = i18n("Wave Map");
            break;
        case TextureUsage::SamplerWaveletMap0:
            name = i18n("Wavelet Map 0");
            break;
        case TextureUsage::SamplerWaveletMap1:
            name = i18n("Wavelet Map 1");
            break;
        case TextureUsage::SamplerWhitecapMap:
            name = i18n("Whitecap Map");
            break;
        default:
            name = i18n("Unknown");
            break;
        }

        auto groupBox = new QGroupBox(name);
        m_texturesLayout->addWidget(groupBox);

        auto layout = new QFormLayout();
        groupBox->setLayout(layout);

        auto texWidget = new TexPart(m_data);
        texWidget->load(physis_gamedata_extract_file(m_data, m_material.textures[i]));
        layout->addRow(i18n("Value:"), texWidget);
    }

    child = nullptr;
    while ((child = m_constantsLayout->takeAt(0)) != nullptr) {
        child->widget()->setParent(nullptr);
        child->widget()->deleteLater();
    }

    for (int i = 0; i < m_material.num_constants; i++) {
        const auto constant = m_material.constants[i];

        QString name = i18n("Unknown %1", QString::number(constant.id));
        if (keys.contains(constant.id)) {
            name = QString::fromLatin1(keys[constant.id]);
        }

        auto groupBox = new QGroupBox(name);
        m_constantsLayout->addWidget(groupBox);

        auto layout = new QFormLayout();
        groupBox->setLayout(layout);

        auto label = new QLabel(QStringLiteral("%1 %2 %3 %4").arg(constant.values[0]).arg(constant.values[1]).arg(constant.values[2]).arg(constant.values[3]));
        layout->addRow(i18n("Value:"), label);
    }
}

#include "moc_mtrlpart.cpp"