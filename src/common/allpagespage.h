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

#ifndef ALLPAGESPAGE_H
#define ALLPAGESPAGE_H

#include <MApplicationPage>
#include <QTimer>
#include <common_export.h>

class AllPagesPagePrivate;
class ThumbProvider;

/*!
 * \class AllPagesPage
 * \brief A class showing all page view
 * The class is a MApplicationPage that shows thumb of all page/slides
 * sheets in a document.
 */

class COMMON_EXPORT AllPagesPage : public MApplicationPage
{
    Q_OBJECT

signals:
    void showPageIndexDefaultZoom(int pageIndex);

public :
    AllPagesPage(const QString& document,const QString& urn, bool spreadSheet, QGraphicsItem *parent = 0);
    virtual ~AllPagesPage();

    /*!
     * \brief Adds page thumb provider.
     * The owner ship is not changed
     * \param newThumbProvider is the new page thumb provider
     */
    void addThumbProvider(ThumbProvider *newThumbProvider);
    void setCurrentPage(int nPageIndex);

    void scrollToCurrentVisiblePage();

protected slots:
    /*!
     * \brief Updates the thumb sizes.
     */
    void updateSizes();

    /*!
     * \brief Marks thumb sizes and count as invalid.
     */
    void pagesChanged();

    /*!
     * \brief Slot is called when thumb provider is deleted.
     */
    void thumbProviderDestroyed();

    /*!
     * \brief Removes all actions from widget lists (and m also makes disconnect).
     */
    void removeActions();

    /*!
    * \brief Handling of short tap
    * \param rect Area of the tap
     */
    virtual void shortTap(QRectF rect, QObject *object);

    /*!
     * \brief Handling of long tap
     * \param rect Area of the tap
     */
    void longTap(QRectF rect);

    /*!
     * \brief Handling of doubble tap
     * \param rect Area of the tap
     */
    void doubleTap(QPointF point);

    /*!
     * \brief The slot is called when a button is called in buttonGroup
     * If the buttonId belongs to this MWidgetAction then this MWidgetAction
     * is triggered and also either the MAction one or two
     */
    void buttonClicked(int buttonId);

    void showFindView();

    void showJumpToolbar();

protected:
    /*!
     * \brief Creates layoyt and policies.
     */
    void createContent();

    /*!
     * \brief Removes all thumb widgets.
     */
    void removeWidgets();

    /*!
     * \brief Adds thumb widgets.
     */
    void addWidgets();

    /*!
     * \brief Checks if thumb widgets are invalid.
     * \return true if thumb widgets are invalid
     */
    bool isDirty();

    /*!
     * \brief Updates thumb widgets if needed and requests thumb provider to update
     * the visible areas.
     */
    void prepareToAppear();

    /*!
     * \brief Gets page index at given point.
     * \returns page index at given point or -1 if none
     */
    int getPageAt(const QPointF &point);

    /*!
     * \Overriding the pinchGesture event to support pinch support
     */
    virtual void pinchGestureEvent(QGestureEvent *event, QPinchGesture *gesture);

    virtual void tapGestureEvent(QGestureEvent *event, QTapGesture *gesture);

    /*!
     * \Overriding the  event to support pinch support
     */
    bool event(QEvent *e);
#if 0
    /*!
     * \Brief zooming out
     */
    void zoomOut();

    /*!
     * \Brief zooming in
     */

    void zoomIn(QPointF center);
#endif

    /*!
     * \brief The connect two buttons with two given action
     * \param one is the action to be connected with first button. If null then spacer is created.
     * \param two is the action to be connected with second button. If null then spacer is created.
     * \returns next button id to be used with the buttonGroup
     */
    void createCombo(MAction *label, MAction *one, MAction *two, MAction *three);

private:
    AllPagesPagePrivate *data;
    QString title;
    int currentPage;
    //Actions which should be changed dynamically are stored here
    //MF - Mark Fav
    //UF - Un Mark Fav
    MAction *appMenuMFAction;
    MAction *appMenuUFAction;
    QString documentUrn;
    bool isSpreadsheet;
};

#endif // ALLPAGESPAGE_H

