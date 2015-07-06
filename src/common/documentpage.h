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
#ifndef DOCUMENTPAGE_H
#define DOCUMENTPAGE_H

#include <QTimer>
#include <QList>
#include <QModelIndex>

#include <MAction>
#include <MApplicationPage>

#include "zoomlevel.h"
#include "actionpool.h"

class QStringListModel;
class QGraphicsSceneMouseEvent;
class QPropertyAnimation;
class TrackerLiveQuery;
class ShareUiInterface;

class MTextEdit;
class MComboBox;
class MDismissEvent;
class MImageWidget;
class MWidgetAction;
class MBanner;

class PageIndicator;
class SearchBar;
class ThumbProvider;
class SlideAnimator;
class JumpToToolbar;
class FindToolbar;
class QuickViewerToolbar;
class ZoomBackground;

#include <common_export.h>

/*!
 * \class DocumentPage
 * \brief The abstract page class for viewing PDF and KOffice documents.
 * Provides common actions and UI such as page indicator.
 */

class COMMON_EXPORT DocumentPage : public MApplicationPage
{
    Q_OBJECT

public :

    //! \brief Search mode enumeration
    enum SearchMode {
        SearchFirst,
        SearchNext,
        SearchPrevious
    };

    /*!
     * \brief The constructor
     * \param parent As in MApplicationPage
     */
    DocumentPage(const QString& document, QGraphicsItem *parent = 0);
    virtual ~DocumentPage();

    void fakeDocumentSaved();

signals:

    /*!
     * \brief Signal is sent if #DocumentPage::loadDocument fails
     * \param file the filename of the document
     * \param reason the reason for failure
     */
    void loadFailed(const QString &file, const QString &reason);

    /*!
     * \brief Signal is sent if #DocumentPage::loadDocument is successful
     * \param file the filename of the loaded document
     */
    void loadSuccess(const QString &file);

    /*!
     * \brief Signal is sent when user wants to close
     */
    void closeDocumentPage();
    /*!
     * \brief Signal which is emited when share UI has to be launched
     */
    void openShare();
    /*!
     * \brief Signal which is emited when document has to be deleted
     */
    void deleteDocument();
    /*!
     * \brief Signal which is emited when a document has to be (un)marked as Favorite
     */
    void toggleFavorite();
    /*!
     * \brief Signal which is emited when document details has to be shown
     */
    void showDetails();

    /*!
     * \brief Signal emited when front page view should be shown
     */
    void showFrontPageView();

    //TODO Below 2 signals are specific to Document so they should be handled in DocumentPage it self
    //But for now as the implementation is in Application Window we are emiting signals.
    //Need to move the implementation to DocumentPage after investigating the effects
    /*!
     * \brief Signal emited when allpages view should be shown
     */
    void showAllPagesView();
    /*!
     * \brief Signal emited when normal view has to be shown
     */
    void showNormalView();
    /*!
     * \brief Signal emited when zoom level is changed by double tap
     */
    void updateZoomLevel(ActionPool::Id item);
    /*!
     * \brief Signal emited when saveas is triggered
     */
    void saveDocumentAs();

    void documentCloseEvent();

    void visibleAreaChanged();

protected:

    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void pinchGestureEvent(QGestureEvent *event, QPinchGesture *gesture);
    virtual void tapAndHoldGestureEvent(QGestureEvent *event, QTapAndHoldGesture *gesture);
    virtual void tapGestureEvent(QGestureEvent *event, QTapGesture *gesture);
    virtual void showPageIndexInternal(int pageIndex) = 0;

public:

    //! Document name which can be sent to Share UI
    QString documentName;
    //! Document urn as given by tracker.
    QString documentUrn;
    //File Title
    QString fileTitle;

    int currentPage;

    //Actions which should be changed dynamically are stored here
    //MF - Mark Fav
    //UF - Un Mark Fav
    MAction *appMenuMFAction;
    MAction *appMenuUFAction;
    /*!
     * \brief Start viewing of given PDF or KOffice document.
     * The signal #DocumentPage::loadFailed or #DocumentPage::loadSuccess is sent when loading is done.
     * \param document the filename of the document
     */
    virtual void loadDocument() = 0;


    /*!
     * \brief Create manu and toolbar items
     */
    virtual void initUI();

    /*!
     * \brief Provides pointer to ThumbProvider
     * The ownership does not changes
     * \returns pointer to ThumbProvider
     */
    virtual ThumbProvider* getThumbProvider() = 0;
    void setEscapeButtonCloseMode();

    void toggleZoomLevel();

    virtual void shortTap(const QPointF &point, QObject *object);
    // TODO use const ref for point
    virtual void longTap(QPointF point);
    virtual void doubleTap(QPointF point);

    virtual void closeEvent(QCloseEvent *event);
    virtual void createFinalContent();

    QRectF visibleRect() const;

    void hidePageIndicator();

    void hideInfoBanner();

    void setDocumentName(const QString &name) { documentName = name; }

    int pageCount() const;

public slots:

    /*!
     * \brief Request for change zooming mode or level.
     * \param level is the new level or mode
     */
    virtual void zoom(ZoomLevel level) = 0;

    /*!
     * \brief Show current page and total amount of pages in the page indicator.
     * Note that if currentpage is less then one it hides page indicator.
     * \param total is the total amount of pages in a document
     * \param currentpage is the currently viewed page in a document
     */
    virtual void setPageCounters(int total, int currentpage);

    /*!
     * \brief Search given text from document.
     * \param mode is the mode of search i.e. first, next or previous
     * \param searchText Text to be searched
     */
    virtual void searchText(DocumentPage::SearchMode mode, const QString &searchText) = 0;

    /*!
     * \brief Clear the Search text from document.
     */
    virtual void clearSearchTexts() = 0;


    /*!
     * \brief Set zoom level to zoom relatively by 200%
     */
    virtual void zoomBy200percent();

    /*!
     * \brief Slot for requesting got opage index
     * \param pageIndex is the page index to be shown
     * \return true if page was in range false otherwise
     */
    bool showPageIndex(int pageIndex);


    /**
     * show that page at the default zoom level
     */
    void showPageIndexDefaultZoom(int pageIndex);

    /*!
     * \brief Slot To update toolbar when screen orientation changes.
     */
    void changeOrientation(const M::Orientation &);

    /*!
     * \brief Slot To update zoom combobox when zoom level changes by double tap
     */
    void updateZoomCombobox(ActionPool::Id item);

    void showNextPage();

    void showPrevPage();

    /*!
     * \brief Slot To update the search if a match was found or not
     */
    void matchesFound(bool found);

    /*!
     * \brief Slot To update the page indicator label for spreadsheet with the current sheet name
     */
    void updatePageIndicatorName(const QString &name);

    virtual void setOpeningProgress(int value);

    void autoHideToolbar();

    /*!
     * \brief If tracker URN is empty, we are in Quick viewer mode else in normal view mode
     */
    bool isQuickViewer() const
    {
        return quickViewer;
    }

    void waitForTrackerIndexing();

protected:
    virtual void pinchStarted(QPointF &center) = 0;
    virtual qreal pinchUpdated(qreal zoomFactor) = 0;
    virtual void pinchFinished(const QPointF &center, qreal scale) = 0;
    virtual QGraphicsWidget *pinchWidget() = 0;

protected slots:

    /*!
     * \brief Set view into full screen mode
     */
    void SetFullscreen();

    /*!
     * \brief Set view into normal screen mode
     */
    void SetNormalscreen();

    /*!
     * \brief Remove action handling
     */
    void removeActions();
    void onClose();
    void onZoomFitToWidth();
    void onZoomFitToPage();
    void onZoom100percent();
#ifdef TESTING_PURPOSE
    void onZoomOut();
    void onZoomIn();
#endif

    void createSearchToolBar();
    void set120percentZoom();

    /*!
     * \brief The slot is called when a button is called in buttonGroup
     * If the buttonId belongs to this MWidgetAction then this MWidgetAction
     * is triggered and also either the MAction one or two
     */
    void zoomButtonClicked(int buttonId);

    /*!
     * \brief The slot is called when a button is called in buttonGroup
     * If the buttonId belongs to this MWidgetAction then this MWidgetAction
     * is triggered and also either the MAction one or two
     */
    void indicatorButtonClicked(int buttonId);

    /*!
     * \brief The slot is called when comboBox is clicked.
     */
    void zoomComboClicked();

    /*!
     * \brief Starts a timer to restore the text for the current index of the zoom comboBox.
     */
    void startTimerToRestoreZoomLevelText();

    /*!
     * \brief Restore the zoom level when comboBox is dismissed.
     */
    void restoreZoomLevelText();

private slots:
    void showInfoBanner(const QString &message);
    void shortTapEvent();
    void sendVisibleAreayChanged();

    void findFirst();
    void findNext();
    void findPrevious();
    void searchTimeout();
    void documentsChanged(QModelIndex topLeft, QModelIndex bottomRight);
    void slotJumpToPage();
    //virtual void changeOrientation(const M::Orientation & orientation);

    void setupBounceAnimation();
    void bounceAnimationFinished();
    void updateViewerType();

private:
    // finish the zoom by pinching
    void finishZoom();
    /*!
     * \brief Add actions to menu and toolbar
     */
    void addActions();
    /*!
     * brief Changes menus based on current status
     * currently only Fav menu item should be changed
     */
    void changeMenus();
    /*!
     * \brief Connect to actions handled by this class
     */
    void connectActions(bool onlyToolbar=false);

    bool searchActive() const;

    void hideFindToolbar();

    bool jumpActive() const;

    /*!
     * \brief The connect two buttons with two given action
     * \param one is the action to be connected with first button. If null then spacer is created.
     * \param two is the action to be connected with second button. If null then spacer is created.
     * \returns next button id to be used with the buttonGroup
     */
    void createCombo(MAction *label, MAction *one, MAction *two, MAction *three);

    void startAutoHideTimer();

    /*!
     * \brief The connect two buttons with two given action
     * \param one is the action to be connected with first button. If null then spacer is created.
     * \param two is the action to be connected with second button. If null then spacer is created.
     * \returns next button id to be used with the buttonGroup
     */
    void createIndicatorCombo(MAction *label, MAction *one, MAction *two, MAction *three);

    /*!
     * \brief Removes unused actions from application page, Called when we are in quick-viewer mode.
     */
    void removeMainViewActions();


protected:
    bool m_pinchInProgress;
    QPointF m_pinchCenter;
    qreal m_endScale; // this is needed when the bounce animation is running
    bool m_blockRecenter;
    bool pageLoaded;
    ActionPool::Id m_defaultZoomLevelAction;
    ZoomLevel m_lastZoom;

private:
    //! The label showing current / tootal page
    PageIndicator       *pageIndicator;
    MBanner             *m_infoBanner;

    // ! Share UI interface
    ShareUiInterface   *shareIf;
    QPointF             m_position;
    QPointF             m_startPoint;
    QTimer              m_shortTapTimer;
    QList<MAction*>     viewToolBarActions;
    QList<MAction*>     searchToolBarActions;
    QList<MAction*>     menuToolBarActions;
    QString             searchstring;
    QTimer              searchTimer;
    bool                searchingTimeout;
    MWidgetAction      *zoomAction;
    MWidgetAction      *indicatorAction;
    MComboBox          *zoomCombobox;
    MComboBox          *indicatorCombobox;
    QString             restoreZoomText;
    bool                searchStarted;
    // ! noMatches set when there is no match for a searched string. Used to restore the text edit when there is a key press
    bool                noMatches;
    QString             textEditStyleName;
    int totalPage;
    TrackerLiveQuery   *liveDocument;
    QPropertyAnimation *bounceAnimation;
    QTimer              m_autoHideTimer;
    JumpToToolbar      *jumpToPageOverlay;
    FindToolbar        *findtoolbar;
    QuickViewerToolbar *quickViewToolbar;
    bool                quickViewer;
    ZoomBackground     *m_pageView;
    qreal               m_lastZoomFactor;
};

#endif // DOCUMENTPAGE_H

