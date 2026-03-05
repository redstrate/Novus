// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shpkpart.h"
#include "../mtrl/knownvalues.h"
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
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#endif

SHPKPart::SHPKPart(physis_SqPackResource *resource, QWidget *parent)
    : QWidget(parent)
    , m_resource(resource)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    shadersTab = new QWidget();

    auto shadersLayout = new QHBoxLayout();
    shadersTab->setLayout(shadersLayout);

    shadersListWidget = new QListWidget();
    connect(shadersListWidget, &QListWidget::activated, this, &SHPKPart::loadShader);
    shadersLayout->addWidget(shadersListWidget);

    shadersTextEdit = new QTextEdit();
    shadersTextEdit->setReadOnly(true);

    shadersScalarListWidget = new QListWidget();
    shadersTextureListWidget = new QListWidget();

    auto shadersTabWidget = new QTabWidget();
    shadersTabWidget->addTab(shadersTextEdit, i18n("Code"));
    shadersTabWidget->addTab(shadersScalarListWidget, i18n("Scalars"));
    shadersTabWidget->addTab(shadersTextureListWidget, i18n("Textures"));
    shadersLayout->addWidget(shadersTabWidget);

    // System
    systemTab = new QScrollArea();
    systemTab->setWidgetResizable(true);

    auto systemTabLayoutHolder = new QWidget();
    systemTab->setWidget(systemTabLayoutHolder);

    systemLayout = new QVBoxLayout();
    systemTabLayoutHolder->setLayout(systemLayout);

    // Scene
    sceneTab = new QScrollArea();
    sceneTab->setWidgetResizable(true);

    auto sceneTabLayoutHolder = new QWidget();
    sceneTab->setWidget(sceneTabLayoutHolder);

    sceneLayout = new QVBoxLayout();
    sceneTabLayoutHolder->setLayout(sceneLayout);

    // Material
    materialTab = new QScrollArea();
    materialTab->setWidgetResizable(true);

    auto materialTabLayoutHolder = new QWidget();
    materialTab->setWidget(materialTabLayoutHolder);

    materialLayout = new QVBoxLayout();
    materialTabLayoutHolder->setLayout(materialLayout);

    // Sub view
    subViewTab = new QScrollArea();
    subViewTab->setWidgetResizable(true);

    auto subViewTabLayoutHolder = new QWidget();
    subViewTab->setWidget(subViewTabLayoutHolder);

    subViewLayout = new QVBoxLayout();
    subViewTabLayoutHolder->setLayout(subViewLayout);

    keysTab = new QTabWidget();
    keysTab->addTab(systemTab, i18n("System"));
    keysTab->addTab(sceneTab, i18n("Scene"));
    keysTab->addTab(materialTab, i18n("Material"));
    keysTab->addTab(subViewTab, i18n("Sub View"));

    nodesTab = new QWidget();
    nodesLayout = new QHBoxLayout();
    nodesTab->setLayout(nodesLayout);

    nodesListWidget = new QListWidget();
    connect(nodesListWidget, &QListWidget::activated, this, &SHPKPart::loadNode);
    nodesLayout->addWidget(nodesListWidget);

    auto nodeSidebarLayout = new QVBoxLayout();
    nodesLayout->addLayout(nodeSidebarLayout);

    nodesSelectorEdit = new QLineEdit();
    nodesSelectorEdit->setReadOnly(true);
    nodeSidebarLayout->addWidget(nodesSelectorEdit);

    nodesPassesTab = new QWidget();
    auto nodesPassesTabLayout = new QHBoxLayout();
    nodesPassesTab->setLayout(nodesPassesTabLayout);

    nodesPassesListWidget = new QListWidget();
    connect(nodesPassesListWidget, &QListWidget::activated, this, &SHPKPart::loadPass);
    nodesPassesTabLayout->addWidget(nodesPassesListWidget);

    nodesPassesFormLayout = new QFormLayout();

    vertexShaderButton = new QPushButton(i18n("Go To Vertex Shader"));
    vertexShaderButton->setEnabled(false);
    nodesPassesFormLayout->addWidget(vertexShaderButton);

    pixelShaderButton = new QPushButton(i18n("Go To Pixel Shader"));
    pixelShaderButton->setEnabled(false);
    nodesPassesFormLayout->addWidget(pixelShaderButton);

    nodesPassesTabLayout->addLayout(nodesPassesFormLayout);

    // System
    nodesSystemTab = new QScrollArea();
    nodesSystemTab->setWidgetResizable(true);

    auto systemTabLayoutNodeHolder = new QWidget();
    nodesSystemTab->setWidget(systemTabLayoutNodeHolder);

    nodesSystemLayout = new QVBoxLayout();
    systemTabLayoutNodeHolder->setLayout(nodesSystemLayout);

    // Scene
    nodesSceneTab = new QScrollArea();
    nodesSceneTab->setWidgetResizable(true);

    auto sceneTabLayoutNodeHolder = new QWidget();
    nodesSceneTab->setWidget(sceneTabLayoutNodeHolder);

    nodesSceneLayout = new QVBoxLayout();
    sceneTabLayoutNodeHolder->setLayout(nodesSceneLayout);

    // Material
    nodesMaterialTab = new QScrollArea();
    nodesMaterialTab->setWidgetResizable(true);

    auto materialTabLayoutNodeHolder = new QWidget();
    nodesMaterialTab->setWidget(materialTabLayoutNodeHolder);

    nodesMaterialLayout = new QVBoxLayout();
    materialTabLayoutNodeHolder->setLayout(nodesMaterialLayout);

    // Sub view
    nodesSubViewTab = new QScrollArea();
    nodesSubViewTab->setWidgetResizable(true);

    auto subViewTabLayoutNodeHolder = new QWidget();
    nodesSubViewTab->setWidget(subViewTabLayoutNodeHolder);

    nodesSubViewLayout = new QVBoxLayout();
    subViewTabLayoutNodeHolder->setLayout(nodesSubViewLayout);

    nodesKeysTabWidget = new QTabWidget();
    nodesKeysTabWidget->addTab(nodesSystemTab, i18n("System"));
    nodesKeysTabWidget->addTab(nodesSceneTab, i18n("Scene"));
    nodesKeysTabWidget->addTab(nodesMaterialTab, i18n("Material"));
    nodesKeysTabWidget->addTab(nodesSubViewTab, i18n("Sub View"));

    nodesTabWidget = new QTabWidget();
    nodesTabWidget->addTab(nodesPassesTab, i18n("Passes"));
    nodesTabWidget->addTab(nodesKeysTabWidget, i18n("Keys"));
    nodeSidebarLayout->addWidget(nodesTabWidget);

    scalarsListWidget = new QListWidget();
    texturesListWidget = new QListWidget();

    pageTabWidget = new QTabWidget();
    pageTabWidget->addTab(shadersTab, i18n("Shaders"));
    pageTabWidget->addTab(keysTab, i18n("Keys"));
    pageTabWidget->addTab(nodesTab, i18n("Nodes"));
    pageTabWidget->addTab(scalarsListWidget, i18n("Scalars"));
    pageTabWidget->addTab(texturesListWidget, i18n("Textures"));
    layout->addWidget(pageTabWidget);
}

void SHPKPart::load(physis_Buffer buffer)
{
    m_shpk = physis_shpk_parse(m_resource->platform, buffer);

    // shaders
    shadersListWidget->clear();

    for (uint32_t i = 0; i < m_shpk.num_vertex_shaders; i++) {
        auto item = new QListWidgetItem();
        item->setText(i18nc("@title:tab", "Vertex Shader %1", i));
        item->setData(Qt::UserRole, 0); // vertex
        item->setData(Qt::UserRole + 1, i); // index
        shadersListWidget->addItem(item);
    }

    for (uint32_t i = 0; i < m_shpk.num_pixel_shaders; i++) {
        auto item = new QListWidgetItem();
        item->setText(i18nc("@title:tab", "Pixel Shader %1", i));
        item->setData(Qt::UserRole, 1); // vertex
        item->setData(Qt::UserRole + 1, i); // index
        shadersListWidget->addItem(item);
    }

    // keys
    const auto addKey = [](QLayout *layout, const Key &key) {
        auto label = new QLabel();
        label->setText(QStringLiteral("<strong>%1</strong><br>Default Value: %2").arg(nameFromCrc(key.id)).arg(nameFromCrc(key.default_value)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    clearLayout(systemLayout);
    for (uint32_t i = 0; i < m_shpk.num_system_keys; i++) {
        addKey(systemLayout, m_shpk.system_keys[i]);
    }
    systemLayout->addStretch();

    clearLayout(sceneLayout);
    for (uint32_t i = 0; i < m_shpk.num_scene_keys; i++) {
        addKey(sceneLayout, m_shpk.scene_keys[i]);
    }
    sceneLayout->addStretch();

    clearLayout(materialLayout);
    for (uint32_t i = 0; i < m_shpk.num_material_keys; i++) {
        addKey(materialLayout, m_shpk.material_keys[i]);
    }
    materialLayout->addStretch();

    clearLayout(subViewLayout);

    // first
    {
        auto label = new QLabel();
        label->setText(QString::fromStdString(nameFromCrc(m_shpk.sub_view_key1_default)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        subViewLayout->addWidget(label);
    }

    // second
    {
        auto label = new QLabel();
        label->setText(QString::fromStdString(nameFromCrc(m_shpk.sub_view_key2_default)));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        subViewLayout->addWidget(label);
    }
    subViewLayout->addStretch();

    // nodes
    nodesListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_nodes; i++) {
        nodesListWidget->addItem(i18nc("@title:tab", "Node %1", i));
    }

    // resources
    scalarsListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_scalar_parameters; i++) {
        scalarsListWidget->addItem(QStringLiteral("%1: %2").arg(i).arg(m_shpk.scalar_parameters[i].name));
    }

    texturesListWidget->clear();
    for (uint32_t i = 0; i < m_shpk.num_texture_parameters; i++) {
        texturesListWidget->addItem(QStringLiteral("%1: %2").arg(i).arg(m_shpk.texture_parameters[i].name));
    }
}

void SHPKPart::loadShader(const QModelIndex &index)
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

        dxvk::DxbcModule module(reader);

        dxvk::DxbcModuleInfo info;
        auto result = module.compile(info, "test");

        spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

        glsl.build_dummy_sampler_for_combined_images();
        glsl.build_combined_image_samplers();

        spirv_cross::CompilerGLSL::Options options;
        options.version = 310;
        options.vulkan_semantics = true;
        glsl.set_common_options(options);

        shadersTextEdit->setText(QLatin1String(glsl.compile().c_str()));

#ifdef HAVE_SYNTAX_HIGHLIGHTING
        // Setup highlighting
        auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(shadersTextEdit->document());
        highlighter->setTheme((shadersTextEdit->palette().color(QPalette::Base).lightness() < 128)
                                  ? repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
                                  : repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

        const auto def = repository.definitionForName(QStringLiteral("GLSL"));
        highlighter->setDefinition(def);
#endif
    } catch (const dxvk::DxvkError &exception) {
        qWarning() << "Failed to load shader:" << exception.message();
    }

    // resources
    shadersScalarListWidget->clear();
    for (uint32_t i = 0; i < shader->num_scalar_parameters; i++) {
        shadersScalarListWidget->addItem(QStringLiteral("%1: %2").arg(shader->scalar_parameters[i].slot).arg(shader->scalar_parameters[i].name));
    }

    shadersTextureListWidget->clear();
    for (uint32_t i = 0; i < shader->num_resource_parameters; i++) {
        shadersTextureListWidget->addItem(QStringLiteral("%1: %2").arg(shader->resource_parameters[i].slot).arg(shader->resource_parameters[i].name));
    }
}

void SHPKPart::loadNode(const QModelIndex &index)
{
    const auto nodeIndex = index.row();
    const physis_SHPKNode *node = &m_shpk.nodes[nodeIndex];

    nodesSelectorEdit->setText(QString::number(node->selector));

    // passes
    nodesPassesListWidget->clear();
    for (int i = 0; i < 16; i++) {
        if (node->pass_indices[i] != 255) {
            auto realPass = node->pass_indices[i];

            auto item = new QListWidgetItem();
            item->setText(i18nc("@title:tab", "%1: %2", i, QString::fromStdString(nameFromCrc(node->passes[realPass].id))));
            item->setData(Qt::UserRole, nodeIndex); // node index
            item->setData(Qt::UserRole + 1, realPass); // pass index
            nodesPassesListWidget->addItem(item);
        } else {
            auto item = new QListWidgetItem();
            item->setText(i18nc("@title:tab", "%1: (No Pass)", i));
            nodesPassesListWidget->addItem(item);
        }
    }

    // keys
    const auto addKey = [](QLayout *layout, const Key upstreamKey, const uint32_t key) {
        QString isDefault;
        if (upstreamKey.default_value == key) {
            isDefault = i18n(" (default)");
        }
        auto label = new QLabel();
        label->setText(QStringLiteral("<strong>%1</strong><br>Value: %2%3").arg(nameFromCrc(upstreamKey.id)).arg(nameFromCrc(key)).arg(isDefault));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    const auto addSubviewKey = [](QLayout *layout, const uint32_t upstreamKeyDefault, const uint32_t key) {
        QString isDefault;
        if (upstreamKeyDefault == key) {
            isDefault = i18n(" (default)");
        }
        auto label = new QLabel();
        label->setText(QStringLiteral("%1%2").arg(nameFromCrc(key)).arg(isDefault));
        label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        layout->addWidget(label);
    };

    clearLayout(nodesSystemLayout);
    for (uint32_t i = 0; i < node->system_key_count; i++) {
        addKey(nodesSystemLayout, m_shpk.system_keys[i], node->system_keys[i]);
    }
    nodesSystemLayout->addStretch();

    clearLayout(nodesSceneLayout);
    for (uint32_t i = 0; i < node->scene_key_count; i++) {
        addKey(nodesSceneLayout, m_shpk.scene_keys[i], node->scene_keys[i]);
    }
    nodesSceneLayout->addStretch();

    clearLayout(nodesMaterialLayout);
    for (uint32_t i = 0; i < node->material_key_count; i++) {
        addKey(nodesMaterialLayout, m_shpk.material_keys[i], node->material_keys[i]);
    }
    nodesMaterialLayout->addStretch();

    clearLayout(nodesSubViewLayout);
    Q_ASSERT(node->subview_key_count < 3);
    for (uint32_t i = 0; i < node->subview_key_count; i++) {
        addSubviewKey(nodesSubViewLayout, i == 0 ? m_shpk.sub_view_key1_default : m_shpk.sub_view_key2_default, node->subview_keys[i]);
    }
    nodesSubViewLayout->addStretch();
}

void SHPKPart::loadPass(const QModelIndex &index)
{
    if (index.data(Qt::DisplayRole).toString().contains(i18n("(No Pass)"))) {
        vertexShaderButton->setEnabled(false);
        pixelShaderButton->setEnabled(false);

        return;
    }

    const auto nodeIndex = index.data(Qt::UserRole).toInt();
    const auto passIndex = index.data(Qt::UserRole + 1).toInt();
    const physis_SHPKNode *node = &m_shpk.nodes[nodeIndex];
    const Pass *pass = &node->passes[passIndex];

    vertexShaderButton->setEnabled(true);
    connect(vertexShaderButton, &QPushButton::clicked, this, [this, pass] {
        goToVertexShader(pass->vertex_shader);
    });

    pixelShaderButton->setEnabled(true);
    connect(pixelShaderButton, &QPushButton::clicked, this, [this, pass] {
        goToPixelShader(pass->pixel_shader);
    });
}

void SHPKPart::goToVertexShader(const int index)
{
    pageTabWidget->setCurrentIndex(0); // shaders tab
    shadersListWidget->setCurrentRow(index);
    Q_EMIT shadersListWidget->activated(shadersListWidget->currentIndex()); // manually activate
}

void SHPKPart::goToPixelShader(const int index)
{
    pageTabWidget->setCurrentIndex(0); // shaders tab

    // Since vertex and pixel shaders share the same list widget currently
    const int numVertexShaders = m_shpk.num_vertex_shaders;
    const int realIndex = numVertexShaders + index;

    shadersListWidget->setCurrentRow(realIndex);
    Q_EMIT shadersListWidget->activated(shadersListWidget->currentIndex()); // manually activate
}

void SHPKPart::clearLayout(QLayout *layout)
{
    QLayoutItem *child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->setParent(nullptr);
            child->widget()->deleteLater();
        }
    }
}

#include "moc_shpkpart.cpp"
