/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "ReportDialog.h"

#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QClipboard>
#include <QApplication>

ReportDialog::ReportDialog(const QString& markdown, QWidget* parent)
    : QDialog(parent)
    , m_markdown(markdown)
{
    setWindowTitle("Benchmark Report");
    resize(600, 500);

    m_browser = new QTextBrowser(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_browser->setMarkdown(markdown);
#else
    m_browser->setPlainText(markdown);
#endif
    m_browser->setOpenExternalLinks(true);

    QPushButton* copyBtn = new QPushButton("Copy Markdown", this);
    QPushButton* closeBtn = new QPushButton("Close", this);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(closeBtn);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_browser);
    layout->addLayout(btnLayout);

    connect(copyBtn, &QPushButton::clicked, this, &ReportDialog::copyMarkdown);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void ReportDialog::copyMarkdown()
{
    QApplication::clipboard()->setText(m_markdown);
}

#include "moc_ReportDialog.cpp"
