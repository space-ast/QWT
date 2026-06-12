/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QDialog>

class QTextBrowser;

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    ReportDialog(const QString& markdown, QWidget* parent = nullptr);

private Q_SLOTS:
    void copyMarkdown();

private:
    QTextBrowser* m_browser;
    QString m_markdown;
};
