// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shpkpart.h"
#include "dxbc_module.h"
#include "dxbc_reader.h"
#include <KLocalizedString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <spirv_glsl.hpp>

SHPKPart::SHPKPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    pageTabWidget = new QTabWidget();
    layout->addWidget(pageTabWidget);
}

void SHPKPart::load(physis_Buffer buffer)
{
    auto shader = physis_parse_shpk(buffer);

    pageTabWidget->clear();

    const auto addShader = [this](physis_Shader shader, const QString &name) {
        auto shaderTextEdit = new QTextEdit();
        shaderTextEdit->setReadOnly(true);

        try {
            dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

            dxvk::DxbcModule module(reader);

            dxvk::DxbcModuleInfo info;
            auto result = module.compile(info, "test");

            spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

            glsl.build_combined_image_samplers();

            spirv_cross::CompilerGLSL::Options options;
            options.version = 310;
            options.vulkan_semantics = true;
            glsl.set_common_options(options);

            shaderTextEdit->setText(QLatin1String(glsl.compile().c_str()));

            pageTabWidget->addTab(shaderTextEdit, name);
        } catch (const std::exception &exception) {
            qWarning() << "Failed to load shader:" << exception.what();
        }
    };

    for (int i = 0; i < shader.num_vertex_shaders; i++) {
        addShader(shader.vertex_shaders[i], i18nc("@title:tab", "Vertex Shader %1", i));
    }

    for (int i = 0; i < shader.num_pixel_shaders; i++) {
        addShader(shader.pixel_shaders[i], i18nc("@title:tab", "Pixel Shader %1", i));
    }
}

#include "moc_shpkpart.cpp"
