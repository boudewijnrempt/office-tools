/*
 * This file is part of Meego Office UI for KOffice
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

//Include QT stuff
#include <QTimer>
#include <QDebug>
#include <QTextCursor>
#include <QTextDocument>
#include <QScrollBar>
#include <QAction>
#include <QTextBlock>
#include <QTextLayout>
#include <QGraphicsWidget>
#include <QSettings>
#include <QDir>

//Include M stuff
#include <MApplication>
#include <MDialog>
#include <MGridLayoutPolicy>
#include <MLabel>
#include <MButton>
#include <MLayout>

//Include Koffice stuff
#include <KoCanvasController.h>
#include <KMimeType>
#include <kmimetypetrader.h>
#include <kparts/componentfactory.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoShapeManager.h>
#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoSelection.h>
#include <KoPADocument.h>
#include <KoPAPageBase.h>
#include <KoPAView.h>
#include <KoFilterManager.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoDocumentInfo.h>

//Include application stuff
#include "officeviewer.h"
#include "definitions.h"
#include "pannablescrollbars.h"

QString OfficeViewer::docOpenError = QString();

OfficeViewer::OfficeViewer(QGraphicsWidget *parent)
    : OfficeViewerBase(parent)
    , m_document(0)
    , isLoaded(false)
    , m_progressUpdater(0)
{
}

OfficeViewer::~OfficeViewer()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void* OfficeViewer::document()
{
    return m_document;
}

void OfficeViewer::loadDocument(const QString &filename, KoProgressProxy *progressProxy)
{
    if (isLoaded) {
        return;
    }

    QString mimetype = KMimeType::findByPath(filename)->name();
    QString error;
    m_document = KMimeTypeTrader::self()->createPartInstanceFromQuery<KoDocument>(
                     mimetype, 0, 0, QString(),
                     QVariantList(), &error);

    if(!error.isEmpty()) {
        qWarning()<<__PRETTY_FUNCTION__<<m_document<<mimetype<<error;
    }

    if(0 != m_document) {
        KUrl url;

        url.setPath(filename);

        m_document->setCheckAutoSaveFile(false);
        m_document->setAutoErrorHandlingEnabled(false);
        m_document->setProgressProxy(progressProxy);
        bool ok = m_document->openUrl(url);
        isLoaded = true;
        m_document->setReadWrite(false);
        qDebug() << "document loaded" << ok << m_document->errorMessage();
        if (!ok && m_document->errorMessage() == "Document is password protected") {
            docOpenError = qtTrId( "qtn_offi_error_pass_protect" );
        }

        QSettings settings((QDir::homePath() + "/.config/office-tools/office-tools.cfg"), QSettings::NativeFormat);
        if (!(settings.value("OldOfficeDialog", QVariant(false)).toBool())) {
            QString generator = m_document->documentInfo()->originalGenerator();
            qDebug() << __PRETTY_FUNCTION__ << generator;
            if (/*generator.contains("Calligra doc Filter/Word ") ||*/ generator.contains("Calligra xls Filter/Excel ")) {
                generator.replace("Calligra xls Filter/Excel ", "");
                generator.replace("Calligra doc Filter/Word ", "");

                if (generator.toInt() < 2002) {
                    MDialog dialog(qtTrId("qtn_offi_unsupported_format"), M::OkButton);
                    dialog.setModal(true);

                    MLayout *layout = new MLayout(dialog.centralWidget());
                    MGridLayoutPolicy *layoutPolicy = new MGridLayoutPolicy(layout);

                    MLabel *dialogText = new MLabel(qtTrId("qtn_offi_older_than_ms_2000"));
                    dialogText->setStyleName("CommonBodyTextInverted");
                    dialogText->setAlignment(Qt::AlignCenter);
                    dialogText->setWordWrap(true);
                    dialogText->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
                    layoutPolicy->addItem(dialogText, 0, 0, 1, 2, Qt::AlignCenter);

                    MButton *chkBox = new MButton();
                    chkBox->setViewType(MButton::checkboxType);
                    chkBox->setCheckable(true);
                    chkBox->setChecked(false);
                    layoutPolicy->addItem(chkBox, 1, 1, Qt::AlignRight);

                    MLabel *chkBoxLabel = new MLabel(qtTrId("qtn_offi_do_not_show_again"));
                    chkBoxLabel->setStyleName("CommonBodyTextInverted");
                    chkBoxLabel->setAlignment(Qt::AlignLeft);
                    chkBoxLabel->setWordWrap(true);
                    chkBoxLabel->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
                    layoutPolicy->addItem(chkBoxLabel, 1, 0, Qt::AlignLeft);

                    dialog.exec();

                    settings.setValue("OldOfficeDialog", QVariant(chkBox->isChecked()));
                    settings.sync();
                }
            }
        }

        emit documentLoaded(ok);
    }
}

QPointF OfficeViewer::textPos(QTextDocument *doc, int position, bool top)
{
    QPointF pos;

    QTextBlock block = doc->findBlock(position);
    QTextLayout *layout = block.layout();
    if (layout) {
        QTextLine line = layout->lineForTextPosition(position - block.position());
        if (line.isValid()) {
            pos = QPointF(line.cursorToX(position - block.position()), line.y());
            if (!top) {
                pos.ry() += line.height();
            }
        }
    }

    qDebug() << __PRETTY_FUNCTION__ << pos;

    return pos;
}


QRectF OfficeViewer::textSelectionRect(KoShape *shape, int position, int length)
{
    KoTextShapeData* tsd = qobject_cast<KoTextShapeData*> (shape->userData());

    if (!tsd || !tsd->document()) {
        return QRectF();
    }

    QTextDocument *doc = tsd->document();
#if 0 //unfortunately the method to get the selectionBoundingBox does not allways return a position
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
    QTextCursor cursor(doc);
    cursor.setPosition(position);
    QRectF r1 = lay->selectionBoundingBox(cursor);
    QTextCursor cursor2(doc);
    cursor2.setPosition(position+length);
    QRectF r2 = lay->selectionBoundingBox(cursor2);
    qDebug() << __PRETTY_FUNCTION__ << r1 << r2 << r1.united(r2).normalized() << position << length;
    //QRectF rect = r1.united(r2).normalized();
#endif

    QRectF rect(textPos(doc, position, true), textPos(doc, position + length, false));
    rect.moveTop(rect.y() - tsd->documentOffset());
    rect = shape->absoluteTransformation(0).mapRect(rect);
    qDebug() << __PRETTY_FUNCTION__ << rect << tsd->documentOffset();

    return rect;
}

void OfficeViewer::updateRange()
{
    if (m_pannableScrollbars) {
        m_pannableScrollbars->updateRange();
    }
}

