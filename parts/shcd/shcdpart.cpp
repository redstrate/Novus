// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shcdpart.h"
#include "dxbc_module.h"
#include "dxbc_reader.h"

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <spirv_glsl.hpp>

#ifdef HAVE_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#endif

SHCDPart::SHCDPart(physis_SqPackResource *resource, QWidget *parent)
    : QWidget(parent)
    , m_resource(resource)
{
    auto layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_shaderTextEdit = new QTextEdit();
    layout->addWidget(m_shaderTextEdit);
}

void SHCDPart::load(physis_Buffer buffer)
{
    m_shcd = physis_shcd_parse(m_resource->platform, buffer);
    if (m_shcd.len == 0) {
        qWarning() << "Failed to parse SHCD!";
        return;
    }

    try {
        dxvk::DxbcReader reader(reinterpret_cast<const char *>(m_shcd.bytecode), m_shcd.len);

        dxvk::DxbcModule module(reader);

        dxvk::DxbcModuleInfo info;
        auto result = module.compile(info, "test");

        spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

        glsl.build_dummy_sampler_for_combined_images();
        glsl.build_combined_image_samplers();

        spirv_cross::CompilerGLSL::Options options;
        options.version = 400; // 400 is so we at least decompile compute shaders
        options.vulkan_semantics = true;
        glsl.set_common_options(options);

        m_shaderTextEdit->setText(QLatin1String(glsl.compile().c_str()));

#ifdef HAVE_SYNTAX_HIGHLIGHTING
        // Setup highlighting
        auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(m_shaderTextEdit->document());
        highlighter->setTheme((m_shaderTextEdit->palette().color(QPalette::Base).lightness() < 128)
                                  ? repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
                                  : repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

        const auto def = repository.definitionForName(QStringLiteral("GLSL"));
        highlighter->setDefinition(def);
#endif
    } catch (const dxvk::DxvkError &exception) {
        qWarning() << "Failed to load shader:" << exception.message();
    }
}

#include "moc_shcdpart.cpp"
