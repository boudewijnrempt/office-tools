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

#ifndef OFFICEVIEWERPRESENTATION_H
#define OFFICEVIEWERPRESENTATION_H

#include "officeviewer.h"
#include "searchresult.h"
#include "slideanimator.h"

#include <QTextCharFormat>
#include <QTimer>

#include <kactioncollection.h>
#include <KoPAViewBase.h>
#include <KoPADocument.h>
#include "documentviewer_export.h"
class QTextDocument;

class KoDocument;

class KoView;

class KoCanvasController;

class KoShape;

class KoPAPageBase;

class KoShapeContainer;

class KoPACanvasItem;

class KoZoomController;

class PresentationView;

class ContextProperty;
/*!
 * \class OfficeViewerPresentation
 * \brief The class is an #OfficeViewer class for showing KPresenter document.
 * The class creates a MPannableViewport where are each slide are shown on
 * it own koffice view.
 */

class DOCUMENTVIEWER_EXPORT OfficeViewerPresentation : public OfficeViewer, public KoPAViewBase
{
    Q_OBJECT

public:

    OfficeViewerPresentation(SlideAnimator *slideAnimator, QGraphicsWidget *parent = 0);
    virtual ~OfficeViewerPresentation();

public: // KoPAViewBase implementation

    virtual KoViewConverter *viewConverter(KoPACanvasBase *canvas);
    virtual KoPACanvasBase * kopaCanvas() const;
    virtual KoPADocument * kopaDocument() const;
    virtual KoZoomController * zoomController() const;
    virtual void doUpdateActivePage(KoPAPageBase * page);
    virtual void setActivePage(KoPAPageBase * page);
    virtual KoPAPageBase* activePage() const;
    virtual void navigatePage(KoPageApp::PageNavigation pageNavigation);
    virtual void setActionEnabled(int actions, bool enable);
    virtual void updatePageNavigationActions();
    virtual void insertPage();
    virtual void pagePaste();
    virtual void editPaste();
    virtual void setShowRulers(bool show);
    virtual QImage * getThumbnail(int page);
    virtual int pageCount()
    {
        if (m_document) {
            KoPADocument* padoc = qobject_cast<KoPADocument*>(m_document);

            return padoc->pageCount();
        }
        return 0;
    }

public: // OfficeView Implementation

    bool createKoWidget();
    QGraphicsLayoutItem *getGraphicsLayoutItem();
    void zoom(const ZoomLevel &newlevel);

    /*!
     * \brief First request for searching given string in document
     * \param searchString is the string to be searched
     */
    void startSearch(const QString & searchString);

//    QList<QImage*> getPreviews();
    void getCurrentVisiblePages(ThumbProvider *thumbProvider);

    virtual void pinchStarted(QPointF &center);
    virtual qreal pinchUpdated(qreal zoomFactor);
    virtual void pinchFinished(const QPointF &center, qreal scale);

    void clearSearchResults();
    void nextWord();
    void previousWord();

public slots:

    void showPage(int pageIndex);


private slots:
    /*!
     * \brief Updates widget sizes for example when orientation changes
     */
    void updateSizes();
    void orientationChanged();
    void resourceChanged(int key, const QVariant &value);

    void updatePageNumbers();
    void setCurrentPage(int pageIndex);

    /*!
     * \brief sets the document offset to the given \param point
     */
    void setDocumentOffset(const QPoint &point);

    void shortTap(const QPointF &point, QObject *object);

    /*!
     * \brief Scale size to either specific width or height
     * \param original Original size
     * \param value desired width/height value
     * \param width Does value mean width (true) or height (false)
     */
    QSizeF scaleTo(const QSizeF &original, qreal value, bool width);

    /*!
     * \brief Slot for handling selection or activation of new tool.
     * \param canvas  the currently active canvas.
     * \param uniqueToolId    a random but unique code for the new tool
     */
    void activeToolChanged(KoCanvasController* canvas, int uniqueToolId);

    /// Go to the previous page
    void goToPreviousPage();
    /// Go to the next page
    void goToNextPage();
    /// Go to the first page
    void goToFirstPage();
    /// Go to the last page
    void goToLastPage();

    // the animation has finshed so use the slide now to show it correctly
    void animationPreviousFinished();
    void animationNextFinished();
    void animationCanceled();

    void tvoutConnected();
    void preventBlanking();

    void topReached(const QPointF &lastPosition);
    void bottomReached(const QPointF &lastPosition);
    void prepareAnimation(const QPointF &lastPosition, KoPageApp::PageNavigation navigation, SlideAnimator::Direction direction);
    void animateSlideTop();
    void animateSlideBottom();

private:

    /*!
    * \brief Scroll to search result and center
    * \param index Index of the search result to scroll to
    */
    void centerToResult(int index);

    /*!
    * \brief Scroll to a certain point within a certain page
    * \param pageIndex index of the page (or slide to be more precise) to scroll
    * to.
    * \param position Relative position (0..1) within the slide to center to, if
    * it is not set, then we scroll so that the top left corner is shown
    * at the top left corner of the screen.
    */
    void scrollTo(int pageIndex, const QPointF &position);

    /**
    * \brief This is a recursive method to find all text shapes from a shapes
    * container
    *
    * KoDocument contains text shapes which have either text or may
    * themselves contain more text shapes. We'll gather them all to a list
    * and then search text from them.
    *
    * \param con KoShapeContainer to look shapes from
    * \param page Page to add to shapes list
    * \param shapes list of found shapes
    * \param docs list of QTextDocuments found within the shapes
    */
    static void findTextShapesRecursive(KoShapeContainer    *con,
                                        KoPAPageBase        *page,
                                        QList<QPair<KoPAPageBase*, KoShape*> >& shapes,
                                        QList<QTextDocument*>& docs);

    /**
    * \brief Find specified text from a list of QTextDocuments
    *
    * \param docs Found QTextDocument list
    * \param shapes Pages and shapes that contained the QTextDocuments
    * \param text Text to search for
    * \return
    */
    void findText(QList<QTextDocument*> docs,
                  QList<QPair<KoPAPageBase*, KoShape*> > shapes,
                  const QString &text);

    /*!
    * \brief Highlight text result with given color
    * \param index Index of the search results to highlight
    * \param current Is the text the current search result (true) or one of the
    * search results (false). Current search result gets highlighted with
    * different color as other search results
    */
    void highlightText(int index, bool current);

    /*!
    * \brief Return the amount of search results we have found
    * \return amount of search results found
    */
    int searchResultCount();

    virtual QSizeF currentDocumentSize();

    /*!
     * \brief Get minimum zoom factor
     *
     * The factor is depending on the orientation
     */
    qreal minimumZoomFactor() const;

private:
    KoPACanvasItem *m_canvasItem;

    KoZoomController *m_zoomController;
    KoPAPageBase *m_currentPage;

    KActionCollection *m_actionCollection;

    int m_currentPageNr;
    int m_pageCount;
    ZoomLevel m_zoomLevel;
    qreal m_lastUserDefinedFactor;
    qreal m_minimumZoomFactor;
    QList<SearchResult> m_searchResults;
    int m_searchIndex;
    QTextCharFormat m_highlight;
    QTextCharFormat m_highlightCurrent;
    ContextProperty *tvout;
    bool tvoutPluggedIn;
    QTimer preventBlankTimer;
    QPoint m_translatedPoint;
    SlideAnimator *m_slideAnimator;
    QPointF m_slideOffset; // this is needed for zoomed sides so that the animation is correct
};

#endif // OFFICEVIEWERPRESENTATION_H
