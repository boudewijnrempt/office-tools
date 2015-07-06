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

#ifndef OFFICEVIEWER_H
#define OFFICEVIEWER_H

#include <KoDocument.h>
#include <KoFilterManager.h>
#include <KoProgressProxy.h>

#include "officeviewerbase.h"
#include "documentviewer_export.h"

#include <actionpool.h>

class MImageWidget;

class KoView;
class KoCanvasController;
class KoShapeContainer;
class KoShape;
class PannableScrollBars;

class DOCUMENTVIEWER_EXPORT OfficeViewer : public OfficeViewerBase
{
    Q_OBJECT

public:

    OfficeViewer(QGraphicsWidget *parent = 0);
    virtual ~OfficeViewer();

    void* document();
    void loadDocument(const QString &filename, KoProgressProxy *progressProxy);

    static QString docOpenError;

signals:
    void updateZoomLevel(ActionPool::Id item);

public slots:
    virtual void updateRange();

protected:

    /**
     * Get point for text position
     */
    QPointF textPos(QTextDocument *doc, int position, bool top);

    /**
     * Get the text selection rectange
     */
    QRectF textSelectionRect(KoShape *shape, int position, int length);

    virtual int pageCount()
    {
        if (m_document) {
            return m_document->pageCount();
        }

        return 0;
    }

    KoDocument *m_document;
    bool isLoaded;
    PannableScrollBars *m_pannableScrollbars;
    // the pinch center in document coordinates
    QPointF m_pinchCenterDocument;

    KoProgressUpdater *m_progressUpdater;
    QPointer<KoUpdater> m_updater;
};

#endif // OFFICEVIEWER_H
