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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "documentlistpage.h"
#include "documentlistmodel.h"
#include "applicationservice.h"
#include "definitions.h"
#include "documentlistitem.h"
#include "OfficeInterface.h"
#include "trackerutils.h"

#include <QGraphicsLinearLayout>
#include <QStringListModel>
#include <QStringList>
#include <QItemSelectionModel>
#include <QApplication>
#include <QProcess>
#include <QDBusMessage>
#include <QTimer>
#include <QFile>
#include <QUrl>
#include <QFutureWatcher>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>

#include <MAbstractCellCreator>
#include <MSortFilterProxyModel>
#include <MLocale>
#include <MAction>
#include <MButton>
#include <MList>
#include <MTextEdit>
#include <MListFilter>
#include <MPannableViewport>
#include <MPopupList>
#include <MWidgetAction>
#include <MComboBox>
#include <MImageWidget>
#include <MButton>
#include <MBanner>
#include <MLabel>
#include <MObjectMenu>
#include <MSceneManager>
#include <MDetailedListItem>
#include <MProgressIndicator>
#include <MApplicationWindow>
#include <MLinearLayoutPolicy>
#include <MLayout>

bool DocumentListPage::pixmapsLoaded = false;

static const char * SortOrderString[] = { "SortByTime", "SortByName", "SortByType" };

class CustomSortFilterProxyModel : public MSortFilterProxyModel
{
protected:
    virtual bool lessThan(const QModelIndex & left, const QModelIndex & right) const {
        if(sourceModel()->hasChildren(left) && sourceModel()->hasChildren(right)) {
            DocumentListModel *ourModel = static_cast<DocumentListModel *>(sourceModel());
            return ourModel->groupLessThan(left,right);
        } else {
            return MSortFilterProxyModel::lessThan(left,right);
        }
    }
};

DocumentListPage::DocumentListPage() :
    proxyModel(0)
    ,list(0)
    ,popuplist(0)
    ,fileLabel(NULL)
    ,nIndex(-1)
    ,longTappedRow(-1)
    ,currentSubview(DocumentListPage::NoSubview)
    ,previousSubview(DocumentListPage::NoSubview)
    ,applicationService(0)
    ,fileLaunchAllowed(true)
    ,usbController(new MeeGo::QmUSBMode(this))
{
    setTitle(qtTrId("qtn_comm_appname_offi"));
    setObjectName("documentlistpage");
    setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    connect(this, SIGNAL(backButtonClicked()), this, SLOT(closeSubview()));
    connect(usbController,SIGNAL(modeChanged(MeeGo::QmUSBMode::Mode)),this, SLOT(usbModeChanged(MeeGo::QmUSBMode::Mode)));
}

void DocumentListPage::usbModeChanged (MeeGo::QmUSBMode::Mode mode)
{
    qDebug() << " **********************USB MODE CHANGED " << mode;
    if (mode == MeeGo::QmUSBMode::MassStorage) {
        //Hack to make the list invisible.
        proxyModelFilter->setFilterRole(DocumentListModel::DocumentListTypeRole);
        proxyModelFilter->setFilterRegExp("Dummy Type to avoid filtering");
        proxyModelFilter->invalidate();
        switchMainView(true,qtTrId("qtn_offi_mass_storage_mode"));
    } else {
        proxyModelFilter->setFilterRole(DocumentListModel::DocumentListLiveFilterRole);
        proxyModelFilter->setFilterRegExp(list->filtering()->editor()->text());
        proxyModelFilter->invalidate();
        if (proxyModelFilter->rowCount()) {
            switchMainView(false);
        } else {
            switchMainView(true,qtTrId("qtn_offi_no_documents"));
        }
    }
}
DocumentListPage::~DocumentListPage()
{
    qDebug() << __PRETTY_FUNCTION__;
    //delete imageLoader;
    delete popuplist;
}


void DocumentListPage::showTextEdit(bool show)
{
    QGraphicsLinearLayout *layout = (QGraphicsLinearLayout *)listViewport->layout();
    layout->setOrientation(Qt::Vertical);
    qDebug() << __PRETTY_FUNCTION__ << show;
    MTextEdit *textEdit = list->filtering()->editor();
    textEdit->setStyleName("CommonSingleInputField");

    if(show && !textEdit->isOnDisplay()) {
        layout->insertItem(0, textEdit);
        textEdit->setVisible(true);
    } else if(textEdit->isOnDisplay()) {
        list->setFocus();
        textEdit->setVisible(false);
        layout->removeAt(0);
        textEdit->setText("");
    }
}

void DocumentListPage::filteringVKB()
{
    // With VKB live filtering text edit is shown when user pans the list up starting from top position
    if(!list->filtering()->editor()->isOnDisplay()) {
        showTextEdit(true);
        list->filtering()->editor()->setFocus();
    }
}


void DocumentListPage::hideEmptyTextEdit()
{
    if(list->filtering()->enabled() && list->filtering()->editor()->text() == "") {
        showTextEdit(false);
    }
}

void DocumentListPage::liveFilteringTextChanged()
{
    if(!list->filtering()->enabled())
        return;

    // With HWKB live filtering text edit is hidden when empty and shown when user enters text
    if(list->filtering()->editor()->text() == "" && list->filtering()->editor()->isOnDisplay())
        QTimer::singleShot(1500, this, SLOT(hideEmptyTextEdit()));
    else if(list->filtering()->editor()->text() != "" && !list->filtering()->editor()->isOnDisplay())
        showTextEdit(true);

    proxyModelFilter->setFilterRole(DocumentListModel::DocumentListLiveFilterRole);
    proxyModelFilter->setFilterRegExp(list->filtering()->editor()->text());
    proxyModelFilter->invalidate();

    if(0 == proxyModelFilter->rowCount()) {
        switchMainView(true, qtTrId("qtn_offi_no_documents"));
    } else {
        switchMainView(false);
    }
}

QString DocumentListPage::highlightText() const
{
    if (!list || !list->filtering() || !list->filtering()->enabled()) {
        return QString();
    }

    return list->filtering()->editor()->text();
}

void DocumentListPage::switchMainView(bool label,QString labelText)
{
    QGraphicsWidget* panel = centralWidget();
    MLayout* layout = (MLayout *) panel->layout();
    MLinearLayoutPolicy *policy = (MLinearLayoutPolicy *)layout->policy();

    // stretch factor is needed as otherwise it does not take all the available space
    if (label) {
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
        if (list->isVisible()) {
            list->hide();
            policy->removeItem(listViewport);
        }
        if (usbController->getMode() == MeeGo::QmUSBMode::MassStorage) {
            fileLabel->setStyleName("CommonEmptyStateTitleKeyword");
            setStyleName("ListPageUSBBackground");
            headerItem->setVisible(false);
        } else {
            fileLabel->setStyleName("CommonEmptyStateTitle");
            setStyleName("ListPageNormalBackground");
            headerItem->setVisible(true);
        }
        fileLabel->setText(labelText.split(QChar(0x9c)).first());
        if (!fileLabel->isVisible()) {
            policy->addItem(fileLabel);
            policy->setStretchFactor(fileLabel,1);
            fileLabel->show();
        }
    }
    else {
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Show);
        if (fileLabel->isVisible()) {
            fileLabel->hide();
            policy->removeItem(fileLabel);
        }
        if (!list->isVisible()) {
            policy->addItem(listViewport);
            policy->setStretchFactor(listViewport,1);
        }
        setStyleName("ListPageNormalBackground");
        headerItem->setVisible(true);
        list->show();
    }
}

void DocumentListPage::createToolbarActions()
{
    shareMultipleToolbarAction = new MAction(qtTrId("qtn_comm_command_share"), this);
    shareMultipleToolbarAction->setLocation(MAction::ToolBarLocation);
    shareMultipleToolbarAction->setObjectName("documentlist_filter_filetype");

    addAction(shareMultipleToolbarAction);
    connect(shareMultipleToolbarAction, SIGNAL(triggered()), this, SLOT(shareMultipleDocumentsActivated()));
    shareMultipleToolbarAction->setVisible(false);

    shareMultipleToolbarCancelAction = new MAction(qtTrId("qtn_comm_cancel"), this);
    shareMultipleToolbarCancelAction->setLocation(MAction::ToolBarLocation);
    shareMultipleToolbarCancelAction->setObjectName("documentlist_filter_filetype");

    addAction(shareMultipleToolbarCancelAction);
    connect(shareMultipleToolbarCancelAction, SIGNAL(triggered()), this, SLOT(closeSubview()));
    shareMultipleToolbarCancelAction->setVisible(false);

    deleteMultipleToolbarAction = new MAction(qtTrId("qtn_comm_command_delete"), this);
    deleteMultipleToolbarAction->setLocation(MAction::ToolBarLocation);
    deleteMultipleToolbarAction->setObjectName("documentlist_filter_filetype");

    addAction(deleteMultipleToolbarAction);
    connect(deleteMultipleToolbarAction, SIGNAL(triggered()), this, SLOT(deleteMultipleDocumentsActivated()));
    deleteMultipleToolbarAction->setVisible(false);

    deleteMultipleToolbarCancelAction = new MAction(qtTrId("qtn_comm_cancel"), this);
    deleteMultipleToolbarCancelAction->setLocation(MAction::ToolBarLocation);
    deleteMultipleToolbarCancelAction->setObjectName("documentlist_filter_filetype");

    addAction(deleteMultipleToolbarCancelAction);
    connect(deleteMultipleToolbarCancelAction, SIGNAL(triggered()), this, SLOT(closeSubview()));
    deleteMultipleToolbarCancelAction->setVisible(false);

    // FIX ME: This is a workaround for AF bug.
    // Icons are not visible if none of the actions are visible in the begining.
    // To be removed after this bug is fixed
    dummyAction = new MAction("", this);
    dummyAction->setLocation(MAction::ToolBarLocation);
    addAction(dummyAction);
    dummyAction->setVisible(true);
}

void DocumentListPage::createObjectMenuActions()
{
    objectMenu = new MObjectMenu(0);
    favAction = new MAction(qtTrId("qtn_comm_command_favorite"), objectMenu);
    favAction->setLocation(MAction::ObjectMenuLocation);
    favAction->setObjectName("documentlist_favorite");
    objectMenu->addAction(favAction);
    connect(favAction, SIGNAL(triggered()), this, SLOT(slotFavourite()));

    nfavAction = new MAction(qtTrId("qtn_comm_command_unmark_favorite"), objectMenu);
    nfavAction->setLocation(MAction::ObjectMenuLocation);
    nfavAction->setObjectName("documentlist_unmark_favorite");
    objectMenu->addAction(nfavAction);
    connect(nfavAction, SIGNAL(triggered()), this, SLOT(slotFavourite()));

    detailsAction = new MAction(qtTrId("qtn_comm_object_details"), objectMenu);
    detailsAction->setLocation(MAction::ObjectMenuLocation);
    detailsAction->setObjectName("documentlist_details");
    objectMenu->addAction(detailsAction);
    connect(detailsAction, SIGNAL(triggered()), this, SLOT(slotDetails()));

    shareAction = new MAction(qtTrId("qtn_comm_command_share"), objectMenu);
    shareAction->setLocation(MAction::ObjectMenuLocation);
    shareAction->setObjectName("documentlist_share");
    objectMenu->addAction(shareAction);
    connect(shareAction, SIGNAL(triggered()), this, SLOT(slotShare()));

    deleteAction = new MAction(qtTrId("qtn_comm_delete"), objectMenu);
    deleteAction->setLocation(MAction::ObjectMenuLocation);
    deleteAction->setObjectName("documentlist_delete");
    objectMenu->addAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(slotDelete()));
}

class MListContentItemCreator : public MAbstractCellCreator<DocumentListItem>
{

public:
    MListContentItemCreator(DocumentListPage *page)
    : m_page(page)
    {}

    void updateCell(const QModelIndex &index, MWidget *cell) const {
        DocumentListItem*listItem = qobject_cast<DocumentListItem *>(cell);

        if(listItem == 0)
            return;

        QVariant listEntry = index.data();
        if (!(listEntry.canConvert<DocumentListEntry>())) {
            return;
        }
        DocumentListEntry entry = listEntry.value<DocumentListEntry>();

        listItem->setTitle(entry.documentName);
        listItem->setSubtitle(qtTrId(entry.documentType.toLatin1().data()));
        listItem->setPage(m_page);

        QPixmap *ico = 0;
        int cat = entry.documentCat;
        switch (cat) {
            case DocumentListModel::DOCUMENT:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-word", QSize(88, 88));
                break;
            case DocumentListModel::OPEN_DOC_TEXT:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-open-document-text", QSize(88, 88));
                break;
            case DocumentListModel::TEXT:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-text", QSize(88, 88));
                break;
            case DocumentListModel::PRESENTATION:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-powerpoint", QSize(88, 88));
                break;
            case DocumentListModel::OPEN_DOC_PPT:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-open-document-presentation", QSize(88, 88));
                break;
            case DocumentListModel::SPREADSHEET:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-excel", QSize(88, 88));
                break;
            case DocumentListModel::OPEN_DOC_XLS:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-open-document-spreadsheet", QSize(88, 88));
                break;
            case DocumentListModel::PDF:
                ico = (QPixmap *)MTheme::pixmap("icon-m-content-pdf", QSize(88, 88));
                break;
            }
        //Sometimes we get NULL ico, to prevent crash lets check it.
        if (ico && !ico->isNull()) {
            listItem->imageWidget()->setPixmap(*(ico));
        }

//        static MLocale locale;
//        listItem->setSideBottomTitle(locale.formatDateTime(entry.lastAccessed,MLocale::DateShort,MLocale::TimeNone));

        if(entry.isFavorite) {
            MImageWidget *image = listItem->sideTopImageWidget();
            image->setPixmap(*MTheme::pixmap("icon-m-common-favorite-mark", QSize(88, 88)));
        } else {
            MImageWidget *image = listItem->sideTopImageWidget();
            image->setPixmap(QPixmap());
        }

        DocumentListPage::pixmapsLoaded = !MTheme::hasPendingRequests();
    }

private:
    DocumentListPage *m_page;
};

void DocumentListPage::loadPicturesInVisibleItems()
{
    //imageLoader->loadPictures(list->firstVisibleItem(), list->lastVisibleItem());
}

void DocumentListPage::setPlainListModel()
{
    qDebug() << __PRETTY_FUNCTION__;
    cellCreator = new MListContentItemCreator(this);
    list->setCellCreator(cellCreator);

    Q_ASSERT(model);
    proxyModel = new CustomSortFilterProxyModel();
    proxyModel->setSortRole(DocumentListModel::DocumentListAccessTimeRole);
    proxyModel->setSourceModel(model);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &))
            , this, SLOT(slotDataChanged()));
    list->setItemModel(proxyModel);
}

void DocumentListPage::refreshList()
{
    list->setSelectionMode(MList::NoSelection);
}

void DocumentListPage::setApplicationService(ApplicationService *service)
{
    applicationService = service;
}

void DocumentListPage::itemClick(const QModelIndex &proxyIndex)
{
    qDebug() << "In DocumentListPage::itemClick...\n";

    QModelIndex tempIndex = proxyModelFilter->mapToSource(proxyIndex);

    QModelIndex index = proxyModel->mapToSource(tempIndex);
    int row = index.row();
    int group = index.parent().isValid()?index.parent().row():-1;
    qDebug() << "List row " << proxyIndex.row() << " first level " << tempIndex.row() << " model level " << index.row();

    if(index.parent().isValid()) {
        qDebug() << "PARENT INDEX ROW IS " << index.parent().row();
    }

    int fd = open(QUrl(QUrl::fromPercentEncoding(model->documentPath(group, row).toUtf8())).path().replace("file://", "").toUtf8().data(), O_RDONLY);
    if (fd < 0) {
        MBanner* infoBanner = new MBanner();
        infoBanner->setStyleName("InformationBanner");
        infoBanner->setTitle(qtTrId("qtn_offi_error_invalid_tracker").split(QChar(0x9c)).first());
        infoBanner->appear((QGraphicsScene *)(applicationWindow()->scene()), MSceneWindow::DestroyWhenDone);
        return;
    }
    ::close(fd);

    bool isEncr = TrackerUtils::Instance().isDocumentEncrypted(model->documentPath(group, row));
    qDebug() << model->documentPath(group, row) << " : " << model->documentPath(group, row) <<" IS ENCRIPTED FLAG IS " << isEncr;

    if(isEncr) {
        MBanner* infoBanner = new MBanner();
        infoBanner->setStyleName("InformationBanner");
        infoBanner->setTitle(qtTrId("qtn_offi_error_pass_protect").split(QChar(0x9c)).first());
        infoBanner->appear((QGraphicsScene *)(applicationWindow()->scene()), MSceneWindow::DestroyWhenDone);
        return;
    }

    // Added to make sure that when launching a file, it is not possible
    // to launch another file immediately.
    if (!fileLaunchAllowed) {
        return;
    } else {
        fileLaunchAllowed = false;
        QTimer::singleShot(5000, this, SLOT(allowFileLaunch()));
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(Service, Path, Interface, QString("launchFile"));
    msg << model->documentPath(group, row);
    bool fileLaunched = false;

    if(applicationService) {
        fileLaunched = applicationService->callMethod(msg);
    }

    if(!fileLaunched) {
        QStringList args;

        args << "--type=m";
        args << "office-tools";
        args << model->documentPath(group, row);
        qDebug() << "------------>>> " <<  __PRETTY_FUNCTION__ <<  "<<<-----------------" << args;
        QProcess::startDetached("invoker", args);
    }
}

void DocumentListPage::allowFileLaunch()
{
    fileLaunchAllowed = true;
}

void DocumentListPage::closeSubview()
{
    if(currentSubview == ShareMultipleView) {
        list->setSelectionMode(MList::NoSelection);
        sortByAppMenuAction->setVisible(true);
        shareMultipleAppMenuAction->setVisible(true);
        deleteMultipleAppMenuAction->setVisible(true);
        markAllAppMenuAction->setVisible(false);
        unmarkAllAppMenuAction->setVisible(false);
        shareMultipleToolbarAction->setVisible(false);
        shareMultipleToolbarCancelAction->setVisible(false);
        connect(list, SIGNAL(itemClicked(const QModelIndex &)), this, SLOT(itemClick(const QModelIndex &)));
        connect(list, SIGNAL(itemLongTapped(const QModelIndex &)), this, SLOT(longTapped(const QModelIndex &)));
    } else if(currentSubview == DeleteMultipleView) {
        list->setSelectionMode(MList::NoSelection);
        sortByAppMenuAction->setVisible(true);
        shareMultipleAppMenuAction->setVisible(true);
        deleteMultipleAppMenuAction->setVisible(true);
        markAllAppMenuAction->setVisible(false);
        unmarkAllAppMenuAction->setVisible(false);

        deleteMultipleToolbarAction->setVisible(false);
        deleteMultipleToolbarCancelAction->setVisible(false);
        connect(list, SIGNAL(itemClicked(const QModelIndex &)), this, SLOT(itemClick(const QModelIndex &)));
        connect(list, SIGNAL(itemLongTapped(const QModelIndex &)), this, SLOT(longTapped(const QModelIndex &)));
    }

    if(0 == proxyModel->rowCount() || (0 == proxyModelFilter->rowCount())) {
        switchMainView(true,qtTrId("qtn_offi_no_documents"));
    } else {
        switchMainView(false);
    }

    currentSubview = previousSubview;

    previousSubview = NoSubview;

    if(currentSubview == NoSubview) {
        nIndex = -1;
        setEscapeMode(MApplicationPageModel::EscapeCloseWindow);
        setTitle(qtTrId("qtn_comm_appname_offi"));
    } else if(currentSubview == FilterView) {
        setTitle(titleList[popuplist->currentIndex().row()]);
    }
}

void DocumentListPage::activateShareMultipleDocumentsView()
{
    previousSubview = currentSubview;
    currentSubview = ShareMultipleView;
    //setEscapeMode(MApplicationPageModel::EscapeManualBack);
    setTitle(qtTrId("qtn_offi_share_documents"));

    //Lets show only the required toolbar actions
    shareMultipleToolbarAction->setDisabled(true);
    shareMultipleToolbarAction->setVisible(true);
    shareMultipleToolbarCancelAction->setVisible(true);
    deleteMultipleToolbarAction->setVisible(false);
    deleteMultipleToolbarCancelAction->setVisible(false);
    dummyAction->setVisible(false);

    //Lets show only the required menu actions
    sortByAppMenuAction->setVisible(false);
    shareMultipleAppMenuAction->setVisible(false);
    deleteMultipleAppMenuAction->setVisible(false);
    markAllAppMenuAction->setVisible(true);
    unmarkAllAppMenuAction->setVisible(false);

    //Lets prepare the list view for multiple selection
    disconnect(list,0,this,0);
    list->setSelectionMode(MList::MultiSelection);

}

void DocumentListPage::activateDeleteMultipleDocumentsView()
{
    previousSubview = currentSubview;
    currentSubview = DeleteMultipleView;
    setTitle(qtTrId("qtn_offi_delete_documents"));

    //Lets show only the required toolbar actions
    deleteMultipleToolbarAction->setDisabled(true);
    deleteMultipleToolbarAction->setVisible(true);
    deleteMultipleToolbarCancelAction->setVisible(true);
    shareMultipleToolbarAction->setVisible(false);
    shareMultipleToolbarCancelAction->setVisible(false);
    dummyAction->setVisible(false);

    //Lets show only the required menu actions
    sortByAppMenuAction->setVisible(false);
    shareMultipleAppMenuAction->setVisible(false);
    deleteMultipleAppMenuAction->setVisible(false);
    markAllAppMenuAction->setVisible(true);
    unmarkAllAppMenuAction->setVisible(false);

    //Lets prepare the list view for multiple selection
    disconnect(list,0,this,0);
    list->setSelectionMode(MList::MultiSelection);
}

void DocumentListPage::shareMultipleDocumentsActivated()
{
    emit openShare();
    //Let us not close the subview here
    //Subview should be closed where actually sharing was possible
    //otherwise lets stay in the same view.
    //closeSubview();
}

void DocumentListPage::deleteMultipleDocumentsActivated()
{
    emit deleteDocuments();
    //Let us not close the subview here as delete has a confirmation dialog
    //subview should be closed if the uses really deletes the files
    //otherwise lets stay in the same view.
    //closeSubview();
}

void DocumentListPage::sortDocumentList(int index)
{
    QDir path;
    path.setPath(QDir::homePath() + "/.config/office-tools/");
    QSettings settings(path.filePath("office-tools.cfg"), QSettings::NativeFormat);

    qDebug() << "" << index;

    switch(index) {
    case 0:
        proxyModel->setSortRole(DocumentListModel::DocumentListAccessTimeRole);
        model->setCurrentGrouping(DocumentListModel::GroupByTime);
        proxyModel->sort(0, Qt::DescendingOrder);
        settings.setValue("SortOrder", SortOrderString[index]);
        break;

    case 1:
        proxyModel->setSortRole(DocumentListModel::DocumentListNameRole);
        model->setCurrentGrouping(DocumentListModel::GroupByName);
        proxyModel->sort(0, Qt::AscendingOrder);
        settings.setValue("SortOrder", SortOrderString[index]);
        break;

    case 2:
        proxyModel->setSortRole(DocumentListModel::DocumentListTypeRole);
        model->setCurrentGrouping(DocumentListModel::GroupByType);
        proxyModel->sort(0, Qt::AscendingOrder);
        settings.setValue("SortOrder", SortOrderString[index]);
        break;
    }
}
void DocumentListPage::setVisibilityOfObjectMenuActions(bool flag)
{
    detailsAction->setVisible(flag);
    shareAction->setVisible(flag);
    deleteAction->setVisible(flag);
}

void DocumentListPage::longTapped(const QModelIndex &proxyIndex)
{
    QModelIndex tempIndex = proxyModelFilter->mapToSource(proxyIndex);
    QModelIndex index = proxyModel->mapToSource(tempIndex);
    longTappedRow = index.row();
    qDebug() << longTappedRow;
    longTappedRowGroup = index.parent().isValid()?index.parent().row():-1;

    setVisibilityOfObjectMenuActions(false);

    if(model->documentIsFavorite(longTappedRowGroup, longTappedRow)) {
        favAction->setVisible(false);
        nfavAction->setVisible(true);
    } else {
        nfavAction->setVisible(false);
        favAction->setVisible(true);
    }

    setVisibilityOfObjectMenuActions(true);

    objectMenu->appear((QGraphicsScene *)(sceneManager()->scene()), MSceneWindow::KeepWhenDone);
}

void DocumentListPage::slotFavourite()
{
    if(longTappedRow == -1) {
        //This is added because of a bug in AF that itemLongTapped is not called.
        //In this case the app might crash in the below steps as longTappedRow is -1
        return;
    }
    qDebug() << __PRETTY_FUNCTION__ << "long tapped row = " << longTappedRow << " Group = " << longTappedRowGroup;
    model->setFavourite(longTappedRowGroup, longTappedRow);
    longTappedRow = -1;
}

void DocumentListPage::slotShare()
{
    emit openShare();
    longTappedRow = -1;
}

void DocumentListPage::slotDelete()
{
    emit deleteDocuments();
    longTappedRow = -1;
}

void DocumentListPage::slotDetails()
{
    emit DocumentDetailsView(model->documentPath(longTappedRowGroup, longTappedRow));
    longTappedRow = -1;
}

void DocumentListPage::createContent()
{
    MApplicationPage::createContent();

    applicationWindow()->setStyleName("FrontPageToolbar");
    applicationWindow()->setNavigationBarOpacity(1.0);

    model = new DocumentListModel();
    connect(model, SIGNAL(liveQueryFinished()), this, SLOT(documentLoadingFinished()));
    initUI();
}

void DocumentListPage::initUI()
{
    setPannable(false);

    MWidget * panel = new MWidget;
    MLayout *layout = new MLayout;
    MLinearLayoutPolicy *policy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    layout->setContentsMargins(0, 0, 0, 0);
    panel->setLayout(layout);

    headerItem = new DocumentHeaderItem(panel);
    headerItem->setTitle(qtTrId("qtn_comm_appname_offi"));
    headerItem->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    policy->addItem(headerItem);

    fileLabel = new MLabel;
    fileLabel->setAlignment(Qt::AlignCenter);
    fileLabel->setWordWrap(true);
    fileLabel->hide();

    listViewport = new MPannableViewport(panel);
    list = new MList(listViewport);
    list->setObjectName("documentlist_list");
    list->setStyleName("OfficeListPageIndex");
    list->setIndexDisplayMode(MList::Floating);
    list->setSelectionMode(MList::NoSelection);
    list->setIndexDisplayMode(MList::Floating);

    setPlainListModel();

    list->installEventFilter(this);
    list->filtering()->setEnabled(true);
    proxyModelFilter = list->filtering()->proxy();
    list->filtering()->editor()->setVisible(false);
//    Uncomment below two lines to get live filtering feature back.
//    connect(list->filtering(), SIGNAL(listPannedUpFromTop()), this, SLOT(filteringVKB()));
//    connect(list->filtering()->editor(), SIGNAL(textChanged()), this, SLOT(liveFilteringTextChanged()));
    listViewport->setWidget(list);
    list->hide();

    setCentralWidget(panel);

    createObjectMenuActions();
    createMenuActions();
    createToolbarActions();
    //Check If we don't have any document to display
    if (usbController->getMode() == MeeGo::QmUSBMode::MassStorage) {
        switchMainView(true,qtTrId("qtn_offi_mass_storage_mode"));
    } else if (proxyModelFilter->rowCount()) {
        switchMainView(false);
    }

    connect(list, SIGNAL(itemClicked(const QModelIndex &)), this, SLOT(itemClick(const QModelIndex &)));
    connect(list, SIGNAL(itemLongTapped(const QModelIndex &)), this, SLOT(longTapped(const QModelIndex &)));
// This code was added to refresh the model after pixmaps are loaded, Guess it is not required any more
// we are invalidating proxymodel when grouping is done, this is causing crash.
#if 0
    //We need to find someother way of repainting rows other than invalidating the proxymodel
//    connect(MTheme::instance(),SIGNAL(pixmapRequestsFinished()), this, SLOT(pixmapLoaded()));
#endif
    connect(list->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection & , const QItemSelection &)),
            SLOT(selectionChanged(const QItemSelection & , const QItemSelection &)));

    const QDir pluginDir("/usr/lib/office-tools/plugins");
    const QStringList plugins = pluginDir.entryList(QDir::Files);

    for (int i = 0; i < plugins.size(); ++i) {
        QPluginLoader test(pluginDir.absoluteFilePath(plugins.at(i)));
        QObject *plug = test.instance();
        if (plug != 0) {
            OfficeInterface* inter = qobject_cast<OfficeInterface*>(plug);
            if (inter) {
                plug->setParent(this);

                if (inter->pluginType() == "generic") { // Other types not supported here
                    MAction *pluginAction = new MAction(inter->pluginName(), this);
                    connect(pluginAction, SIGNAL(triggered()), plug, SLOT(emitOpenSignal()));
                    connect(plug, SIGNAL(openMe(OfficeInterface*)), this, SLOT(openPlugin(OfficeInterface*)));
                    pluginAction->setLocation(MAction::ApplicationMenuLocation);
                    addAction(pluginAction);
                }
            }
            else {
                delete plug;
            }
        }
    }
}

void DocumentListPage::documentLoadingFinished()
{
    if((proxyModel && (proxyModel->rowCount() == 0)) || usbController->getMode() == MeeGo::QmUSBMode::MassStorage) {
        if(usbController->getMode() == MeeGo::QmUSBMode::MassStorage) {
            proxyModelFilter->setFilterRole(DocumentListModel::DocumentListTypeRole);
            proxyModelFilter->setFilterRegExp("Dummy Type to avoid filtering");
            proxyModelFilter->invalidate();
            switchMainView(true,qtTrId("qtn_offi_mass_storage_mode"));
        } else {
            switchMainView(true,qtTrId("qtn_offi_no_documents"));
        }
        disconnect(list->filtering(), SIGNAL(listPannedUpFromTop()), this, SLOT(filteringVKB()));
        disconnect(list->filtering()->editor(), SIGNAL(textChanged()), this, SLOT(liveFilteringTextChanged()));
    } else {
        switchMainView(false);
        model->setGrouped(true);
        list->setShowGroups(true);
    }
    connect(model, SIGNAL(updateListPage()), this, SLOT(slotUpdateListPage()));
    connect(model, SIGNAL(listDeleteCompleted()), this, SLOT(listUpdateFinished()));
}

void DocumentListPage::slotUpdateListPage()
{
    if (proxyModel && proxyModel->rowCount() == 0) {
        switchMainView(true,qtTrId("qtn_offi_no_documents"));
    } else {
        if (!list->isVisible()) {
            switchMainView(false);
            model->setCurrentGrouping(DocumentListModel::GroupByTime);
            model->setGrouped(true);
            list->setShowGroups(true);
        }
    }
}

void DocumentListPage::pixmapLoaded()
{
    if(!pixmapsLoaded) {
        proxyModel->invalidate();

        if(MTheme::hasPendingRequests()) {
            qWarning() << "We Still have some pending requests to Load pixmaps";
        } else {
            pixmapsLoaded = true;
        }
    }
}

void DocumentListPage::favoriteSettingsChanged()
{
    proxyModel->invalidate();
}

void DocumentListPage::createMenuActions()
{
    sortByAppMenuAction = new MWidgetAction(this);
    sortByAppMenuAction->setLocation(MAction::ApplicationMenuLocation);

    QStringList list;
    list << qtTrId("qtn_comm_sort_by_time") << qtTrId("qtn_comm_sort_by_name") << qtTrId("qtn_offi_sort_by_type");
    MComboBox *comboBox = new MComboBox;
    comboBox->setObjectName("documentlist_sort_list");
    comboBox->addItems(list);
    comboBox->setIconVisible(false);
    comboBox->setTitle(qtTrId("qtn_comm_sort_by"));

    QDir path;
    path.setPath(QDir::homePath() + "/.config/office-tools/");
    QSettings settings(path.filePath("office-tools.cfg"), QSettings::NativeFormat);
    int index = 0;
    QVariant sortOrder = settings.value("SortOrder", "SortByTime");
    for (unsigned int i = 0; i < sizeof(SortOrderString) / sizeof(*SortOrderString); ++i) {
        qDebug() << __PRETTY_FUNCTION__ << SortOrderString[i];
        if (sortOrder == SortOrderString[i]) {
            qDebug() << __PRETTY_FUNCTION__ << "sorted by" << SortOrderString[i];
            index = i;
            break;
        }
    }
    sortDocumentList(index);
    comboBox->setCurrentIndex(index);
    sortByAppMenuAction->setWidget(comboBox);
    addAction(sortByAppMenuAction);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(sortDocumentList(int)));

    shareMultipleAppMenuAction = new MAction(qtTrId("qtn_offi_share_documents"), this);
    shareMultipleAppMenuAction->setLocation(MAction::ApplicationMenuLocation);
    shareMultipleAppMenuAction->setObjectName("documentlist_share_multiple");
    connect(shareMultipleAppMenuAction, SIGNAL(triggered()), this, SLOT(activateShareMultipleDocumentsView()));
    addAction(shareMultipleAppMenuAction);

    deleteMultipleAppMenuAction = new MAction(qtTrId("qtn_offi_delete_documents"), this);
    deleteMultipleAppMenuAction->setObjectName("documentlist_delete_multiple");
    deleteMultipleAppMenuAction->setLocation(MAction::ApplicationMenuLocation);
    connect(deleteMultipleAppMenuAction, SIGNAL(triggered()), this, SLOT(activateDeleteMultipleDocumentsView()));
    addAction(deleteMultipleAppMenuAction);

    markAllAppMenuAction = new MAction(qtTrId("qtn_comm_command_mark_all"), this);
    markAllAppMenuAction->setObjectName("documentlist_mark_all");
    markAllAppMenuAction->setLocation(MAction::ApplicationMenuLocation);
    connect(markAllAppMenuAction, SIGNAL(triggered()), this, SLOT(markAllActivated()));
    addAction(markAllAppMenuAction);
    markAllAppMenuAction->setVisible(false);

    unmarkAllAppMenuAction = new MAction(qtTrId("qtn_comm_command_unmark_all"), this);
    unmarkAllAppMenuAction->setObjectName("documentlist_unmark_all");
    unmarkAllAppMenuAction->setLocation(MAction::ApplicationMenuLocation);
    connect(unmarkAllAppMenuAction, SIGNAL(triggered()), this, SLOT(unmarkAllActivated()));
    addAction(unmarkAllAppMenuAction);
    unmarkAllAppMenuAction->setVisible(false);
}

void DocumentListPage::slotDataChanged()
{
    if(proxyModel && (proxyModel->rowCount() == 0)) {
        switchMainView(true,qtTrId("qtn_offi_no_documents"));
    } else {
        switchMainView(false);
    }
}

int DocumentListPage::getlongTappedRow()
{
    return longTappedRow;
}
DocumentListModel *DocumentListPage::getDocumentModel()
{
    return model;
}

DocumentListPage::DocumentListSubview DocumentListPage::getCurrentSubview()
{
    return currentSubview;
}

QModelIndexList DocumentListPage::getSelectedItemIndexes()
{
    return list->selectionModel()->selection().indexes();
}

QStringList DocumentListPage::getSelectedUrns()
{
    QStringList uris;
    QModelIndexList selectedItems = list->selectionModel()->selection().indexes();
    foreach(QModelIndex curInd,selectedItems) {
        QModelIndex tempIndex = proxyModelFilter->mapToSource(curInd);
        QModelIndex index = proxyModel->mapToSource(tempIndex);
        uris << model->documentUri((index.parent().isValid()?index.parent().row():-1), index.row());
    }

    return uris;
}

QStringList DocumentListPage::getSelectedPaths()
{
    QStringList paths;
    QModelIndexList selectedItems = list->selectionModel()->selection().indexes();
    foreach(QModelIndex curInd,selectedItems) {
        QModelIndex tempIndex = proxyModelFilter->mapToSource(curInd);
        QModelIndex index = proxyModel->mapToSource(tempIndex);
        paths << model->documentPath((index.parent().isValid()?index.parent().row():-1), index.row());
    }

    return paths;
}

int DocumentListPage::getFileCount()
{
    if(!model->isGrouped())
        return proxyModelFilter->rowCount();

    int hdrcnt = proxyModelFilter->rowCount();
    int fileCount = 0;

    for(int i = 0; i < hdrcnt; i++) {
        QModelIndex index = proxyModelFilter->index(i,0);
        fileCount += proxyModelFilter->rowCount(index);
    }

    return fileCount;
}

QString DocumentListPage::getLongTappedRowUrn()
{
    if(longTappedRow != -1)
        return model->documentUri(longTappedRowGroup, longTappedRow);
    else
        return QString();
}

QString DocumentListPage::getLongTappedRowPath()
{
    if(longTappedRow != -1)
        return model->documentPath(longTappedRowGroup, longTappedRow);
    else
        return QString();
}

void DocumentListPage::markAllActivated()
{
    list->selectionModel()->clearSelection();
    int rowCount = proxyModelFilter->rowCount();

    for(int i= 0; i < rowCount; i++) {
        QModelIndex index = proxyModelFilter->index(i,0);

        if(proxyModelFilter->hasChildren(index)) {
            for(int j = 0; j < proxyModelFilter->rowCount(index); j++) {
                QModelIndex cindex = proxyModelFilter->index(j,0,index);
                list->selectItem(cindex);
            }
        } else {
            list->selectItem(index);
        }
    }
}

void DocumentListPage::unmarkAllActivated()
{
    list->selectionModel()->clearSelection();
}

bool DocumentListPage::eventFilter(QObject *obj, QEvent *event)
{
    if((currentSubview == ShareMultipleView || currentSubview == DeleteMultipleView) &&
       obj == list &&
       event->type() == QEvent::GraphicsSceneContextMenu) {
        qDebug() << "EATING OFF THE EVENT as we are in Share/Delete subview";
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void DocumentListPage::openPlugin(OfficeInterface *plugin)
{
    MApplicationPage *pluginView = plugin->createView();
    pluginView->appear(scene(), MSceneWindow::DestroyWhenDismissed);
}

void DocumentListPage::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if(currentSubview != ShareMultipleView && currentSubview != DeleteMultipleView) {
        return;
    }
    bool state= list->selectionModel()->selection().indexes().count() > 0;
    if(currentSubview == ShareMultipleView) {
        shareMultipleToolbarAction->setDisabled(!state);
    } else if (currentSubview == DeleteMultipleView) {
        deleteMultipleToolbarAction->setDisabled(!state);
    }

    if(getFileCount() == list->selectionModel()->selection().indexes().count()) {
        markAllAppMenuAction->setVisible(false);
        unmarkAllAppMenuAction->setVisible(true);
    } else {
        markAllAppMenuAction->setVisible(true);
        unmarkAllAppMenuAction->setVisible(false);
    }
}

bool DocumentListPage::showSpinner() const
{
    return (fileLaunchAllowed && list->selectionMode() != MList::MultiSelection);
}
void DocumentListPage::docsDeleted(QStringList list)
{
    model->notifyOnDeleteFinished(list);
    headerItem->showSpinner();
}
void DocumentListPage::listUpdateFinished()
{
    headerItem->hideSpinner();
}
