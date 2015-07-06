#ifndef DOCUMENTVIEWER_H
#define DOCUMENTVIEWER_H

#include "ViewerInterface.h"
#include "documentpage.h"
#include "applicationwindow.h"
#include "documentviewer_export.h"

class DOCUMENTVIEWER_EXPORT DocumentViewer : public QObject, public ViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(ViewerInterface)

public:
    DocumentViewer();
    DocumentPage *createDocumentPage(ApplicationWindow::DocumentType documentType, const QString &filePath);

};


#endif // DOCUMENTVIEWER_H
