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

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include <MApplicationWindow>
#include <MSceneManager>

#include "documentpage.h"

class ApplicationService;
class AllPagesPage;
class DocumentListPage;
class ShareUiInterface;
class DocumentDetailView;
class QSettings;
class ViewerInterface;
class MBanner;
#include <common_export.h>

/*!
* \brief Main window of the application
*
*/

class COMMON_EXPORT ApplicationWindow : public QObject
{
    Q_OBJECT

public:

    /*!
     * \brief The view mode
     */
    enum ViewMode {
        //! No view
        NoView,
        //! Normal page/slide/sheet view
        NormalView,
        //! All pages/slides/sheets view a.k.k thumb view
        AllPagesView,
        //!Document Detail View
        DocDetailView
    };
    /*!
    * \brief Supported document types
    *
    */
    enum DocumentType {
        DOCUMENT_WORD,
        DOCUMENT_PRESENTATION,
        DOCUMENT_PDF,
        DOCUMENT_SPREADSHEET,
        DOCUMENT_UNKOWN
    };

public:
    ApplicationWindow(MApplicationWindow *p_appWindow);
    ~ApplicationWindow();

private Q_SLOTS:
    void slotPageChanged(MApplicationPage *page);

public slots:
    /*!
    * \brief Open a document
    * \param fileName filename of the document to open
    * \return true if success
    */
    bool OpenFile(const QString& fileName);
    void OpenListPage();

    /*!
    * \brief Opens a new instance with the given document or popups the existing instance which
    * already has the given document opened
    * \param fileName filename of the document to open
    * \return true if success
    */
    bool launchFile(const QString& fileName);

    /*!
    * \brief Slot to catch signal sent by document loader when opening a
    * document fails
    *
    * \param file Filename of the document that failed to open
    * \param reason Reason we failed to open the document
    */
    void loadFailed(const QString &file, const QString &reason);

    /*!
    * \brief Slot to catch signal sent by documentloader when document was
    * opened
    *
    * \param file Filename of the document that was opened
    */
    void loadSuccess(const QString &file);

    /*!
     * \brief Slot for closing active document page
     */
    void closeDocumentPage();

    /*!
     * \brief Slot for showing page or all pages view
     */
    void showPages();

    /*!
     * \brief Slot for hiding pages (releasing memory)
     */
    void hidePages();

    void DocumentDetailsView(QString documentPath);

    void slotShare();

    void slotDelete();

    void deleteConfirmationYes();

    void deleteConfirmationNo();

    void slotSaveAs();
    
    void documentPageDisplayEntered();

    void exitApplication();

public:
    /*!
     * \brief Static method for checking file family based on the mime type.
     *
     * \param fileName is the file name
     * \return The "family" of family
     */
    static DocumentType checkMimeType(const QString &fileName);

    /*!
     * \brief Just a static wrapper to get visibleSceneSize.
     *
     * \return The visible scene size
     */
    static QSize visibleSize();

    static QSize visibleSizeCorrect();

    /*!
     * \brief Just a static wrapper to get visibleSceneSize.
     *
     * \param orientation The orientation of wanted visibleSceneSize
     * \return The visible scene size in givem orientation
     */
    static QSize visibleSize(M::Orientation orientation);

    /*!
     * \brief Just a static wrapper to get MSceneManager.
     *
     * \return The MSceneManager
     */
    static MSceneManager* GetSceneManager();

    /*!
     * \brief sets whether the instance is first instance of office-tools.
     *
     * \param isFirstInstance to say whether the instance is first instance
     */
    void setIsFirstInstance(bool isFirstInstance);

    /*!
     * \brief to check whether the first instance of office-tools is running.
     *
     * \return true or false to say whether the first instance of office-tools is running
     */
    bool firstInstanceRunning();

    /*!
     * \brief sets the application service object.
     *
     * \param service the application service object
     */
    void setApplicationService(ApplicationService *service);

protected:
    /*!
     * \brief Connetcs actions handle by this class
     */
    void addActions();

protected slots:
    /*!
     * \brief Remove action handling
     */
    void removeActions();
    /*!
     * \brief Shows document in normal view
     */
    void showNormalView();

    void toNormalView();

    /*!
     * \brief Shows document in all pages view
     */
    void showAllPagesView();

    void slotFavourite();

    void DocumentDetailsView();

    void documentSaved(const QString &fileName);

private slots:
    void slotOpenDocument();
    void showFrontPageView();

private:
    Q_DISABLE_COPY(ApplicationWindow)

    void showBannerInformation(const QString &message);

    /*!
    * \brief Current document page
    *
    */
    DocumentPage *page;
    /*!
     * \brief If page created loaded (once)
     *
     */
    bool pageLoaded;
    /*!
     * \brief The all pages view page
     *
     */
    AllPagesPage *allPagesPage;
    DocumentDetailView *docDetail;
    ViewMode     viewMode;
    DocumentListPage *pageList;
    ShareUiInterface   *shareIf;
    bool showPagesPending;
    QString filePath;
    QStringList pathsToDelete;
    MApplicationWindow *appWindow;
    bool firstInstance;
    bool frontPageLaunched;
    QSettings *officeToolsSettings;
    ApplicationService *applicationService;
    ViewerInterface *viewerInterface;
    MBanner *infoBanner;
};

#endif
