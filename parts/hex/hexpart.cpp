// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#include "hexpart.h"

#include "document/buffer/qmemoryrefbuffer.h"
#include "qhexview.h"
#include "texteditor.h"

#include <KLocalizedString>
#include <KSeparator>
#include <QComboBox>
#include <QFormLayout>
#include <QStackedWidget>
#include <QStringConverter>

HexPart::HexPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QFormLayout();
    setLayout(layout);

    const auto stackedWidget = new QStackedWidget();

    m_hexView = new QHexView();
    m_hexView->setReadOnly(true);
    stackedWidget->addWidget(m_hexView);

    m_textEdit = new TextEditor();
    stackedWidget->addWidget(m_textEdit);

    const auto pageComboBox = new QComboBox();
    pageComboBox->addItem(i18n("Bytes"));
    pageComboBox->addItem(i18n("Text"));
    connect(pageComboBox, &QComboBox::activated, stackedWidget, &QStackedWidget::setCurrentIndex);

    layout->addRow(i18n("View As"), pageComboBox);
    layout->addRow(new KSeparator());
    layout->addRow(stackedWidget);
}

void HexPart::loadFile(const physis_Buffer buffer)
{
    m_hexView->setDocument(QHexDocument::fromMemory<QMemoryRefBuffer>(reinterpret_cast<char *>(buffer.data), buffer.size, this));
    m_textEdit->setBuffer(QByteArray::fromRawData(reinterpret_cast<const char *>(buffer.data), buffer.size));
}

#include "moc_hexpart.cpp"
