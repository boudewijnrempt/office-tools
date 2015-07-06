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

#ifndef DOCUMENTLISTPAGE_H
#define DOCUMENTLISTPAGE_H

#include <MApplicationPage>

#include <QSortFilterProxyModel>
#include <QtConcurrentRun>
#include <MObjectMenu>

class ApplicationService;
class DocumentListModel;
class DocumentSortedModel;
class DocumentImageLoader;
class MList;
class MLabel;
class MPopupList;
class MAction;
class MWidgetAction;
class MListContentItemCreator;
class MSortFilterProxyModel;
class MProgressIndicator;
class DocumentHeaderItem;
class OfficeInterface;

#include <common_export.h>
#include <qmusbmode.h>

class COMMON_EXPORT DocumentListPage: public MApplicationPage
{
    Q_OBJECT

public:
    enum DocumentListPageSorting {
        None,
        Ascending,
        Descending
    };
    typedef enum {
        NoSubview,
        FilterView,
        ShareMultipleView,
        DeleteMultipleView
    } DocumentListSubview;

    DocumentListPage();
    virtual ~DocumentListPage();

    virtual void createContent();

    void createActions();
    DocumentListModel *getDocumentModel();
    int getlongTappedRow();
    DocumentListPage::DocumentListSubview getCurrentSubview();
    QModelIndexList getSelectedItemIndexes();
    QStringList getSelectedUrns();
    QStringList getSelectedPaths();
    QString getLongTappedRowUrn();
    QString getLongTappedRowPath();
    int getFileCount();
    void setApplicationService(ApplicationService *service);

    /**
     * Returns the viewport responsible for panning the page
     */
    MPannableViewport * pannableViewport() {  return listViewport; }

    /**
     * Should the spinner shown when clicking an item in the list
     */
    bool showSpinner() const;

    /**
     * Get highlight text
     */
    QString highlightText() const;

    static bool pixmapsLoaded;

public slots:
    void usbModeChanged(MeeGo::QmUSBMode::Mode mode);
    void loadPicturesInVisibleItems();
    void setPlainListModel();
    void itemClick(const QModelIndex &index);
    void sortDocumentList(int index);
    void closeSubview();
    void favoriteSettingsChanged();
    void activateShareMultipleDocumentsView();
    void activateDeleteMultipleDocumentsView();
    void shareMultipleDocumentsActivated();
    void deleteMultipleDocumentsActivated();
    void markAllActivated();
    void unmarkAllActivated();
    void refreshList();
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void docsDeleted(QStringList list);
    void listUpdateFinished();

signals:
    void openShare();
    void deleteDocuments();
    void DocumentDetailsView(QString documentPath);

private slots:
    void slotFavourite();
    void slotDetails();
    void slotShare();
    void slotDelete();
    void longTapped(const QModelIndex &);
    void slotDataChanged();
    void pixmapLoaded();
    void liveFilteringTextChanged();
    void filteringVKB();
    void hideEmptyTextEdit();
    void initUI();
    void openPlugin(OfficeInterface *plugin);
    void allowFileLaunch();
    void documentLoadingFinished();
    void slotUpdateListPage();

private:
    void createToolbarActions();
    void createObjectMenuActions();
    void createMenuActions();
    void setVisibilityOfObjectMenuActions(bool flag);
    bool eventFilter(QObject *obj, QEvent *event);

    void showTextEdit(bool show);
    void switchMainView(bool label,QString labelText = QString());

    DocumentListModel *model;
    MSortFilterProxyModel *proxyModel;
    MSortFilterProxyModel *proxyModelFilter;
    DocumentImageLoader *imageLoader;
    MList *list;
    MObjectMenu *objectMenu;
    MListContentItemCreator *cellCreator;
    MPopupList *popuplist;
    MAction *shareMultipleToolbarAction;
    MAction *shareMultipleToolbarCancelAction;
    MAction *deleteMultipleToolbarAction;
    MAction *deleteMultipleToolbarCancelAction;
    MAction *favAction;
    MAction *nfavAction;
    MAction *detailsAction;
    MAction *shareAction;
    MAction *deleteAction;
    MWidgetAction *sortByAppMenuAction;
    MAction *shareMultipleAppMenuAction;
    MAction *deleteMultipleAppMenuAction;
    MAction *markAllAppMenuAction;
    MAction *unmarkAllAppMenuAction;
    MAction *dummyAction; //FIXME: see documentlistpage.cpp:181
    MLabel *fileLabel;
    bool sortingOrder;
    int nIndex;
    int longTappedRow;
    int longTappedRowGroup;
    QStringList titleList;
    DocumentListSubview currentSubview;
    DocumentListSubview previousSubview;
    ApplicationService *applicationService;
    MProgressIndicator *m_openingProgressIndicator;
    bool fileLaunchAllowed;
    MPannableViewport *listViewport;
    DocumentHeaderItem *headerItem;
    MeeGo::QmUSBMode *usbController;
};

#endif
