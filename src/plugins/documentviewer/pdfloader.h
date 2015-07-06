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

#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QTimer>
#include <QVector>
//#include <QList>
//#include <QObject>
//#include <QImage>
#include <poppler-qt4.h>
#include "documentviewer_export.h"
namespace Poppler
{

class Document;
}

class PdfLoaderPrivate;

class QGraphicsItem;

class QGraphicsScene;

class PdfImage;

class PdfLoaderThread;
class PdfImageCache;
class PdfPageWidget;

/*!
 * \class PdfLoader
 * \brief The class provides loading of PDF pages.
 *  The class loads full page images with given scale from poppler and
 * stores the images into #PdfImageCache.
 */

class DOCUMENTVIEWER_EXPORT PdfLoader : public QObject
{
    Q_OBJECT

signals:
    /*!
     * \brief The signal is sent page is changed
     * \param total is the total amount of pages in a document
     * \param current is the currently viewed page in a document (starting from 1)
     */
    void pageChanged(int total, int current);

    /*!
     * \brief The signal is sent if storages image have wrong scale
     * \param pageIndex the pages index having 'wrong' scale
     */
    /*!
     * \brief The request is sent to chack if there pages to loaded in background
     */
    void loadNeighborPagesRequest();

    /*!
     * \brief The request is sent when background loading should stop
     */
    void stopBackGroundLoading();

    void thumbnailLoaded(int pageIndex);

public:
    static const int DPIPerInch = 72;

    PdfLoader(QObject *parent = 0);
    virtual ~PdfLoader();

    /*!
     * \brief Loads document into poppler and creates a #PdfImageCache.
     * \param filename file to be loaded
     * \param mDocument pointer to reference of poppler doc.
     * \return false if loading fails
     */
    bool load(const QString &filename,Poppler::Document* &mDocument);

    /*!
     * \brief Getter for page count
     * \return Number of pages in in document or zero if not loaded.
     */
    int  numberOfPages() const;

    /*!
     * \brief Getter for page size
     * \param pageIndex Page index of requested page size.
     * \return Orginal size of page
     */
    QSize pageSize(int pageIndex) const;

    /*!
     * \brief Sets current page.
     * Sends #PdfLoader::pageChanged signal when current page changes.
     * \param pageIndex the new page index
     */
    void setCurrentPage(int pageIndex);

    /*!
     * \brief Get current page.index
     * \return current page index or -1
     */
    int getCurrentPageIndex() const;

    /*!
     * \brief Gets list of pages that area visible in given scene area.
     * \param rect the scene area
     * \return list of page indexies in area. List is empty is widgets are hidden.
     */
    QList<int>  getItemsAtSceneArea(QRectF rect) const;

    /*!
     * \brief Gives pointer to scene.
     * The scene is needed to searching items
     * \param graphicsScene the scene
     */
    void setScene(const QGraphicsScene *graphicsScene);

    /*!
     * \brief Provide name of #BasePageWidget objects so that they can be searched.
     * \param newName The name of #BasePageWidget objects
     */
    void setWidgetName(const QString & newName);

    /*!
     * \brief Gets list of text in page.
     * \param pageIndex the pages index
     * \return List of TextBox*. The caller must delete the TextBox objects.
     */
    QList<Poppler::TextBox*> getTextBoxList(int pageIndex);

    /*!
     * \brief Gets list of links in page.
     * \param pageIndex the pages index
     * \return List of Link*. The caller must delete the Link objects.
     */
    QList<Poppler::Link *> getLinks(int pageIndex);

    /*!
    * \brief Set the pointer for highlight text QHash.
    * \param highlights pointer to refer the highlighted data.
    */
    void setHighlightData(const  QHash<int, QList<QRectF> > *highlights);

    /*!
    * \brief Get the pointer for highlight text QHash.
    * \return return the pointer of the highlighted QHash data .
    */
    const QHash<int, QList<QRectF> >* getHighlightData();

    /*!
     * \brief Get the index of current highlight text.
     * \return return the index number of the highlighted QHash data .
     */
    void getCurrentHighlight(int &pageIndex, int &highlightPostion);

    /*!
     * \brief Set the index of current highlight text.
     * \param chl number of current highlight text.
     */
    void  setCurrentHighlight(int pageIndex, int highlightPostion);

    /**
     * 
     */
    QImage getPageImage(int pageIndex, qreal scale, PdfPageWidget *pageWidget);

    /**
     *
     */
    QImage getThumbnail(int pageIndex, qreal scale);

public slots:
    /*!
     * \brief Removes page images that are not used currently.
     */
    void removeUnused();

    /*!
    * \brief The slot for loading pages near by current page
    */
    void loadNeighborPages();

    void updatePage(PdfPageWidget *pageWidget);

protected:
    /*!
     * \brief Clears the pdf data
     */
    void clear();

    /*!
     * \brief Getter for given page data
     * \param pageIndex The page index
     */
    PdfLoaderPrivate * getPageData(int pageIndex) const;

    /*!
     * \brief Gets area the is little bit bigger the visible scene size
     * \return The area little bit bigger the visible scene
     */
    QRectF getNeighborRect();


private:
    Poppler::Document*          document;
    QVector<PdfLoaderPrivate*>  dataItems;
    int                         currentPageIndex;
    const QGraphicsScene        *scene;
    QString                     widgetName;
    const QHash<int, QList<QRectF> >   *mHighlights;
    int    highlightPageIndex;
    int       highlightCurrentPosition;
    PdfLoaderThread             *thread;
    PdfImageCache *m_imageCache;
};

#endif // PDFLOADER_H
