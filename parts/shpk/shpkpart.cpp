// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shpkpart.h"
#include "../../common/include/knownvalues.h"
#include "dxbc_module.h"
#include "dxbc_reader.h"

#include <KLocalizedString>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <spirv_glsl.hpp>

#ifdef HAVE_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#endif

SHPKPart::SHPKPart(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    m_shadersTab = new QWidget();

    auto shadersLayout = new QHBoxLayout();
    m_shadersTab->setLayout(shadersLayout);

    m_shadersListWidget = new QListWidget();
    connect(m_shadersListWidget, &QListWidget::activated, this, &SHPKPart::loadShader);
    shadersLayout->addWidget(m_shadersListWidget);

    m_shadersTextEdit = new QTextEdit();
    m_shadersTextEdit->setReadOnly(true);
    m_shadersTextEdit->setFontFamily(QStringLiteral("monospace"));

    m_shadersScalarListWidget = new QListWidget();
    m_shadersTextureListWidget = new QListWidget();

    auto shadersTabWidget = new QTabWidget();
    shadersTabWidget->addTab(m_shadersTextEdit, i18n("Code"));
    shadersTabWidget->addTab(m_shadersScalarListWidget, i18n("Scalars"));
    shadersTabWidget->addTab(m_shadersTextureListWidget, i18n("Textures"));
    shadersLayout->addWidget(shadersTabWidget);

    // System
    m_systemTab = new QScrollArea();
    m_systemTab->setWidgetResizable(true);

    auto systemTabLayoutHolder = new QWidget();
    m_systemTab->setWidget(systemTabLayoutHolder);

    m_systemLayout = new QVBoxLayout();
    systemTabLayoutHolder->setLayout(m_systemLayout);

    // Scene
    m_sceneTab = new QScrollArea();
    m_sceneTab->setWidgetResizable(true);

    auto sceneTabLayoutHolder = new QWidget();
    m_sceneTab->setWidget(sceneTabLayoutHolder);

    m_sceneLayout = new QVBoxLayout();
    sceneTabLayoutHolder->setLayout(m_sceneLayout);

    // Material
    m_materialTab = new QScrollArea();
    m_materialTab->setWidgetResizable(true);

    auto materialTabLayoutHolder = new QWidget();
    m_materialTab->setWidget(materialTabLayoutHolder);

    m_materialLayout = new QVBoxLayout();
    materialTabLayoutHolder->setLayout(m_materialLayout);

    // Sub view
    m_subViewTab = new QScrollArea();
    m_subViewTab->setWidgetResizable(true);

    auto subViewTabLayoutHolder = new QWidget();
    m_subViewTab->setWidget(subViewTabLayoutHolder);

    m_subViewLayout = new QVBoxLayout();
    subViewTabLayoutHolder->setLayout(m_subViewLayout);

    m_keysTab = new QTabWidget();
    m_keysTab->addTab(m_systemTab, i18n("System"));
    m_keysTab->addTab(m_sceneTab, i18n("Scene"));
    m_keysTab->addTab(m_materialTab, i18n("Material"));
    m_keysTab->addTab(m_subViewTab, i18n("Sub View"));

    m_nodesTab = new QWidget();
    m_nodesLayout = new QHBoxLayout();
    m_nodesTab->setLayout(m_nodesLayout);

    m_nodesListWidget = new QListWidget();
    connect(m_nodesListWidget, &QListWidget::activated, this, &SHPKPart::loadNode);
    m_nodesLayout->addWidget(m_nodesListWidget);

    auto nodeSidebarLayout = new QVBoxLayout();
    m_nodesLayout->addLayout(nodeSidebarLayout);

    m_nodesSelectorEdit = new QLineEdit();
    m_nodesSelectorEdit->setReadOnly(true);
    nodeSidebarLayout->addWidget(m_nodesSelectorEdit);

    m_nodesPassesTab = new QWidget();
    auto nodesPassesTabLayout = new QHBoxLayout();
    m_nodesPassesTab->setLayout(nodesPassesTabLayout);

    m_nodesPassesListWidget = new QListWidget();
    connect(m_nodesPassesListWidget, &QListWidget::activated, this, &SHPKPart::loadPass);
    nodesPassesTabLayout->addWidget(m_nodesPassesListWidget);

    m_nodesPassesFormLayout = new QFormLayout();

    m_vertexShaderButton = new QPushButton(i18n("Go To Vertex Shader"));
    m_vertexShaderButton->setEnabled(false);
    m_nodesPassesFormLayout->addWidget(m_vertexShaderButton);

    m_pixelShaderButton = new QPushButton(i18n("Go To Pixel Shader"));
    m_pixelShaderButton->setEnabled(false);
    m_nodesPassesFormLayout->addWidget(m_pixelShaderButton);

    nodesPassesTabLayout->addLayout(m_nodesPassesFormLayout);

    // System
    m_nodesSystemTab = new QScrollArea();
    m_nodesSystemTab->setWidgetResizable(true);

    auto systemTabLayoutNodeHolder = new QWidget();
    m_nodesSystemTab->setWidget(systemTabLayoutNodeHolder);

    m_nodesSystemLayout = new QVBoxLayout();
    systemTabLayoutNodeHolder->setLayout(m_nodesSystemLayout);

    // Scene
    m_nodesSceneTab = new QScrollArea();
    m_nodesSceneTab->setWidgetResizable(true);

    auto sceneTabLayoutNodeHolder = new QWidget();
    m_nodesSceneTab->setWidget(sceneTabLayoutNodeHolder);

    m_nodesSceneLayout = new QVBoxLayout();
    sceneTabLayoutNodeHolder->setLayout(m_nodesSceneLayout);

    // Material
    m_nodesMaterialTab = new QScrollArea();
    m_nodesMaterialTab->setWidgetResizable(true);

    auto materialTabLayoutNodeHolder = new QWidget();
    m_nodesMaterialTab->setWidget(materialTabLayoutNodeHolder);

    m_nodesMaterialLayout = new QVBoxLayout();
    materialTabLayoutNodeHolder->setLayout(m_nodesMaterialLayout);

    // Sub view
    m_nodesSubViewTab = new QScrollArea();
    m_nodesSubViewTab->setWidgetResizable(true);

    auto subViewTabLayoutNodeHolder = new QWidget();
    m_nodesSubViewTab->setWidget(subViewTabLayoutNodeHolder);

    m_nodesSubViewLayout = new QVBoxLayout();
    subViewTabLayoutNodeHolder->setLayout(m_nodesSubViewLayout);

    m_nodesKeysTabWidget = new QTabWidget();
    m_nodesKeysTabWidget->addTab(m_nodesSystemTab, i18n("System"));
    m_nodesKeysTabWidget->addTab(m_nodesSceneTab, i18n("Scene"));
    m_nodesKeysTabWidget->addTab(m_nodesMaterialTab, i18n("Material"));
    m_nodesKeysTabWidget->addTab(m_nodesSubViewTab, i18n("Sub View"));

    m_nodesTabWidget = new QTabWidget();
    m_nodesTabWidget->addTab(m_nodesPassesTab, i18n("Passes"));
    m_nodesTabWidget->addTab(m_nodesKeysTabWidget, i18n("Keys"));
    nodeSidebarLayout->addWidget(m_nodesTabWidget);

    m_scalarsListWidget = new QListWidget();
    m_texturesListWidget = new QListWidget();

    m_pageTabWidget = new QTabWidget();
    m_pageTabWidget->addTab(m_shadersTab, i18n("Shaders"));
    m_pageTabWidget->addTab(m_keysTab, i18n("Keys"));
    m_pageTabWidget->addTab(m_nodesTab, i18n("Nodes"));
    m_pageTabWidget->addTab(m_scalarsListWidget, i18n("Scalars"));
    m_pageTabWidget->addTab(m_texturesListWidget, i18n("Textures"));
    layout->addWidget(m_pageTabWidget);
}

void SHPKPart::load(const Platform platform, const physis_Buffer buffer)
{
    m_shpk = physis_shpk_parse(platform, buffer);

    // shaders
    m_shadersListWidget->clear();

    for (uint32_t i = 0; i < m_shpk.num_vertex_shaders; i++) {
        const auto item = new QListWidgetItem();
        item->setText(i18nc("@title:tab", "Vertex Shader %1", i));
        item->setData(Qt::UserRole, 0); // vertex
        item->setData(Qt::UserRole + 1, i); // index
        m_shadersListWidget->addItem(item);
    }

    for (uint32_t i = 0; i < m_shpk.num_pixel_shaders; i++) {
        const auto item = new QListWidgetItem();
        item->setText(i18nc("@title:tab", "Pixel Shader %1", i));
        item->setData(Qt::UserRole, 1); // vertex
        item->setData(Qt::UserRole + 1, i); // index
        m_shadersListWidget->addItem(item);
    }

    // keys
    const auto addKey = [](QLayout *layout, const Key &key) {
        const auto label = new QLabel();
        label->setText(QStringLiteral("<strong>%1</strong><br>Default Value: %2").arg(nameFromCrc(key.id)).arg(nameFromCrc(key.default_value)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    clearLayout(m_systemLayout);
    for (uint32_t i = 0; i < m_shpk.num_system_keys; i++) {
        addKey(m_systemLayout, m_shpk.system_keys[i]);
    }
    m_systemLayout->addStretch();

    clearLayout(m_sceneLayout);
    for (uint32_t i = 0; i < m_shpk.num_scene_keys; i++) {
        addKey(m_sceneLayout, m_shpk.scene_keys[i]);
    }
    m_sceneLayout->addStretch();

    clearLayout(m_materialLayout);
    for (uint32_t i = 0; i < m_shpk.num_material_keys; i++) {
        addKey(m_materialLayout, m_shpk.material_keys[i]);
    }
    m_materialLayout->addStretch();

    clearLayout(m_subViewLayout);

    // first
    {
        const auto label = new QLabel();
        label->setText(QString::fromStdString(nameFromCrc(m_shpk.sub_view_key1_default)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        m_subViewLayout->addWidget(label);
    }

    // second
    {
        const auto label = new QLabel();
        label->setText(QString::fromStdString(nameFromCrc(m_shpk.sub_view_key2_default)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        m_subViewLayout->addWidget(label);
    }
    m_subViewLayout->addStretch();

    // nodes
    m_nodesListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_nodes; i++) {
        m_nodesListWidget->addItem(i18nc("@title:tab", "Node %1", i));
    }

    // resources
    m_scalarsListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_scalar_parameters; i++) {
        m_scalarsListWidget->addItem(QStringLiteral("%1: %2").arg(i).arg(m_shpk.scalar_parameters[i].name));
    }

    m_texturesListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_texture_parameters; i++) {
        m_texturesListWidget->addItem(QStringLiteral("%1: %2").arg(i).arg(m_shpk.texture_parameters[i].name));
    }
}

void SHPKPart::loadShader(const QModelIndex &index) const
{
    const auto shaderType = index.data(Qt::UserRole).toInt();
    const auto shaderIndex = index.data(Qt::UserRole + 1).toInt();

    const physis_Shader *shader = nullptr;
    if (shaderType == 0) {
        shader = &m_shpk.vertex_shaders[shaderIndex];
    } else if (shaderType == 1) {
        shader = &m_shpk.pixel_shaders[shaderIndex];
    } else {
        Q_UNREACHABLE();
    }

    Q_ASSERT(shader);

    try {
        dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader->bytecode), shader->len);

        const dxvk::DxbcModule module(reader);

        const dxvk::DxbcModuleInfo info;
        auto result = module.compile(info, "test");

        spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

        glsl.build_dummy_sampler_for_combined_images();
        glsl.build_combined_image_samplers();

        spirv_cross::CompilerGLSL::Options options;
        options.version = 310;
        options.vulkan_semantics = true;
        glsl.set_common_options(options);

        m_shadersTextEdit->setText(QLatin1String(glsl.compile().c_str()));

#ifdef HAVE_SYNTAX_HIGHLIGHTING
        // Setup highlighting
        const auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(m_shadersTextEdit->document());
        highlighter->setTheme(m_shadersTextEdit->palette().color(QPalette::Base).lightness() < 128
                                  ? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
                                  : m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

        const auto def = m_repository.definitionForName(QStringLiteral("GLSL"));
        highlighter->setDefinition(def);
#endif
    } catch (const dxvk::DxvkError &exception) {
        qWarning() << "Failed to load shader:" << exception.message();
    }

    // resources
    m_shadersScalarListWidget->clear();
    for (uint32_t i = 0; i < shader->num_scalar_parameters; i++) {
        m_shadersScalarListWidget->addItem(QStringLiteral("%1: %2").arg(shader->scalar_parameters[i].slot).arg(shader->scalar_parameters[i].name));
    }

    m_shadersTextureListWidget->clear();
    for (uint32_t i = 0; i < shader->num_resource_parameters; i++) {
        m_shadersTextureListWidget->addItem(QStringLiteral("%1: %2").arg(shader->resource_parameters[i].slot).arg(shader->resource_parameters[i].name));
    }
}

void SHPKPart::loadNode(const QModelIndex &index) const
{
    const auto nodeIndex = index.row();
    const physis_SHPKNode *node = &m_shpk.nodes[nodeIndex];

    m_nodesSelectorEdit->setText(QString::number(node->selector));

    // passes
    m_nodesPassesListWidget->clear();
    for (int i = 0; i < 16; i++) {
        if (node->pass_indices[i] != 255) {
            const auto realPass = node->pass_indices[i];

            const auto item = new QListWidgetItem();
            item->setText(i18nc("@title:tab", "%1: %2", i, QString::fromStdString(nameFromCrc(node->passes[realPass].id))));
            item->setData(Qt::UserRole, nodeIndex); // node index
            item->setData(Qt::UserRole + 1, realPass); // pass index
            m_nodesPassesListWidget->addItem(item);
        } else {
            const auto item = new QListWidgetItem();
            item->setText(i18nc("@title:tab", "%1: (No Pass)", i));
            m_nodesPassesListWidget->addItem(item);
        }
    }

    // keys
    const auto addKey = [](QLayout *layout, const Key upstreamKey, const uint32_t key) {
        QString isDefault;
        if (upstreamKey.default_value == key) {
            isDefault = i18n(" (default)");
        }
        const auto label = new QLabel();
        label->setText(QStringLiteral("<strong>%1</strong><br>Value: %2%3").arg(nameFromCrc(upstreamKey.id)).arg(nameFromCrc(key)).arg(isDefault));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    const auto addSubviewKey = [](QLayout *layout, const uint32_t upstreamKeyDefault, const uint32_t key) {
        QString isDefault;
        if (upstreamKeyDefault == key) {
            isDefault = i18n(" (default)");
        }
        const auto label = new QLabel();
        label->setText(QStringLiteral("%1%2").arg(nameFromCrc(key)).arg(isDefault));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    clearLayout(m_nodesSystemLayout);
    for (uint32_t i = 0; i < node->system_key_count; i++) {
        addKey(m_nodesSystemLayout, m_shpk.system_keys[i], node->system_keys[i]);
    }
    m_nodesSystemLayout->addStretch();

    clearLayout(m_nodesSceneLayout);
    for (uint32_t i = 0; i < node->scene_key_count; i++) {
        addKey(m_nodesSceneLayout, m_shpk.scene_keys[i], node->scene_keys[i]);
    }
    m_nodesSceneLayout->addStretch();

    clearLayout(m_nodesMaterialLayout);
    for (uint32_t i = 0; i < node->material_key_count; i++) {
        addKey(m_nodesMaterialLayout, m_shpk.material_keys[i], node->material_keys[i]);
    }
    m_nodesMaterialLayout->addStretch();

    clearLayout(m_nodesSubViewLayout);
    Q_ASSERT(node->subview_key_count < 3);
    for (uint32_t i = 0; i < node->subview_key_count; i++) {
        addSubviewKey(m_nodesSubViewLayout, i == 0 ? m_shpk.sub_view_key1_default : m_shpk.sub_view_key2_default, node->subview_keys[i]);
    }
    m_nodesSubViewLayout->addStretch();
}

void SHPKPart::loadPass(const QModelIndex &index)
{
    if (index.data(Qt::DisplayRole).toString().contains(i18n("(No Pass)"))) {
        m_vertexShaderButton->setEnabled(false);
        m_pixelShaderButton->setEnabled(false);

        return;
    }

    const auto nodeIndex = index.data(Qt::UserRole).toInt();
    const auto passIndex = index.data(Qt::UserRole + 1).toInt();
    const physis_SHPKNode *node = &m_shpk.nodes[nodeIndex];
    const Pass *pass = &node->passes[passIndex];

    m_vertexShaderButton->setEnabled(true);
    connect(m_vertexShaderButton, &QPushButton::clicked, this, [this, pass] {
        goToVertexShader(pass->vertex_shader);
    });

    m_pixelShaderButton->setEnabled(true);
    connect(m_pixelShaderButton, &QPushButton::clicked, this, [this, pass] {
        goToPixelShader(pass->pixel_shader);
    });
}

void SHPKPart::goToVertexShader(const int index) const
{
    m_pageTabWidget->setCurrentIndex(0); // shaders tab
    m_shadersListWidget->setCurrentRow(index);
    Q_EMIT m_shadersListWidget->activated(m_shadersListWidget->currentIndex()); // manually activate
}

void SHPKPart::goToPixelShader(const int index) const
{
    m_pageTabWidget->setCurrentIndex(0); // shaders tab

    // Since vertex and pixel shaders share the same list widget currently
    const int numVertexShaders = m_shpk.num_vertex_shaders;
    const int realIndex = numVertexShaders + index;

    m_shadersListWidget->setCurrentRow(realIndex);
    Q_EMIT m_shadersListWidget->activated(m_shadersListWidget->currentIndex()); // manually activate
}

void SHPKPart::clearLayout(QLayout *layout)
{
    const QLayoutItem *child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->setParent(nullptr);
            child->widget()->deleteLater();
        }
    }
}

#include "moc_shpkpart.cpp"
