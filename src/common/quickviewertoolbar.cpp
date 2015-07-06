#include "quickviewertoolbar.h"
#include "applicationwindow.h"
#include "documentpage.h"

#include <MButton>
#include <MApplication>

#include <QGraphicsGridLayout>

#include <float.h>

QuickViewerToolbar::QuickViewerToolbar(QGraphicsItem *parent)
    : MOverlay(parent)
{
    DocumentPage *page = static_cast<DocumentPage *>(parent);
    if (!page) {
        return;
    }
    setStyleName("CommonOverlaySheetHeaderPanel");

    saveButton = new MButton(this);
    saveButton->setText(qtTrId("qtn_comm_save"));
    saveButton->setStyleName("CommonSheetHeaderButtonAccentInverted");
    connect(saveButton, SIGNAL(clicked()), page, SIGNAL(saveDocumentAs()));

    closeButton = new MButton(this);
    closeButton->setText(qtTrId("qtn_comm_command_close"));
    closeButton->setStyleName("CommonSheetHeaderButtonInverted");
    connect(closeButton, SIGNAL(clicked()), page, SIGNAL(documentCloseEvent()));

    toolbarLayout = new QGraphicsGridLayout(this);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(0);

    toolbarLayout->addItem(closeButton, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    toolbarLayout->addItem(createSpacer(), 0, 1);
    toolbarLayout->addItem(saveButton, 0, 2, Qt::AlignTop | Qt::AlignRight);
    setPos(0, 0);
    setZValue(FLT_MAX);
    connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChanged(const M::Orientation &)),
            this, SLOT(updatePosition(const M::Orientation &)));
}

QGraphicsWidget *QuickViewerToolbar::createSpacer()
{
    QGraphicsWidget *spacer = new QGraphicsWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return spacer;
}

void QuickViewerToolbar::updatePosition(const M::Orientation &orientation)
{
    QSize size = MApplication::activeWindow()->visibleSceneSize(orientation);
    resize(size.width(), preferredSize().height());
}

