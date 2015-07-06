#include "documentviewer.h"
#include "officepage.h"
#include "pdfpage.h"

#include <QDebug>

DocumentViewer::DocumentViewer()
{
}


DocumentPage *DocumentViewer::createDocumentPage(ApplicationWindow::DocumentType documentType, const QString &filePath)
{
    DocumentPage *page = 0;

    switch(documentType) {
#ifndef NO_KOFFICE
    case ApplicationWindow::DOCUMENT_WORD:
        // fall-through
    case ApplicationWindow::DOCUMENT_PRESENTATION:
        // fall-through
    case ApplicationWindow::DOCUMENT_SPREADSHEET:
        // Create new document page
        page = new OfficePage(filePath);
        break;
#endif
    case ApplicationWindow::DOCUMENT_PDF:
        page = new PdfPage(filePath);
        break;
    case ApplicationWindow::DOCUMENT_UNKOWN:
        // fall-through
    default:
        qWarning() << "UnRecognized file type !!!";
        break;
    }
    return page;
}

Q_EXPORT_PLUGIN2(office-tools-viewer, DocumentViewer)
