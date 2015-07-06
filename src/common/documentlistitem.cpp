#include "documentlistitem.h"

#include "documentlistpage.h"
#include <MGridLayoutPolicy>
#include <MLabel>
#include <MImageWidget>
#include <MProgressIndicator>
#include <MLayout>
#include <MLocale>

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QGraphicsSceneResizeEvent>

DocumentHeaderItem::DocumentHeaderItem(QGraphicsWidget *parent)
    : MBasicListItem(MBasicListItem::SingleTitle, parent)
{
    setStyleName("CommonHeaderPanel");
    titleLabelWidget()->setStyleName("CommonHeaderInverted");
    progressSpinner = new MProgressIndicator(this, MProgressIndicator::spinnerType);
    progressSpinner->setStyleName("CommonViewHeaderSpinnerInverted");
    //progressSpinner->setStyleName("CommonThumbnailSpinner");
    progressSpinner->setVisible(false);
}
void DocumentHeaderItem::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    progressSpinner->setPos(event->newSize().width() - progressSpinner->size().width(), 
            (event->newSize().height() - progressSpinner->size().height())/2);
}

void DocumentHeaderItem::showSpinner()
{
    progressSpinner->setVisible(true);
    progressSpinner->setUnknownDuration(true);
}

void DocumentHeaderItem::hideSpinner()
{
    progressSpinner->setVisible(false);
    progressSpinner->reset();
    
}

DocumentListItem::DocumentListItem()
    : MListItem(),
    layout(NULL),
    layoutPolicy(NULL),
    titleLabel(NULL),
    subtitleLabel(NULL),
    icon(NULL),
    sideTopIcon(NULL),
    sideBottomSubtitleLabel(NULL),
    itemSpinner(NULL),
    m_page(0)
{
    setStyleName("CommonPanel");
    setLayout(createLayout());
}

MLayout *DocumentListItem::createLayout()
{
    layout = new MLayout(this);
    layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(0, 0, 0, 0);

    layoutPolicy = new MGridLayoutPolicy(layout);
    layoutPolicy->setContentsMargins(0, 0, 0, 0);
    layoutPolicy->setSpacing(0);

    layoutPolicy->addItem(imageWidget(), 0, 0, 3, 1);

    layoutPolicy->addItem(titleWidget(), 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignBottom);
    layoutPolicy->addItem(sideTopImageWidget(), 0, 3, Qt::AlignRight | Qt::AlignBottom);

    layoutPolicy->addItem(subtitleWidget(), 1, 1, Qt::AlignLeft | Qt::AlignBottom);
    layoutPolicy->addItem(sideBottomSubtitleWidget(), 1, 2, 1, 2, Qt::AlignRight | Qt::AlignBottom);

    // we need to set the size to the preferred size of the item in the columns
    qreal spinnerWidth = spinner()->preferredSize().width();
    layoutPolicy->setColumnFixedWidth(3, spinnerWidth );

    qreal sbslWidth = sideBottomSubtitleLabel->preferredSize().width() - spinnerWidth;
    sbslWidth = sbslWidth >= 0 ? sbslWidth : 0;
    layoutPolicy->setColumnFixedWidth(2, sbslWidth);

    layoutPolicy->addItem(new QGraphicsWidget, 2, 1, 1, 2);

    layout->setPolicy(layoutPolicy);
    connect(this, SIGNAL(clicked()), SLOT(showSpinner()));

    return layout;
}

MLabel *DocumentListItem::titleWidget()
{
    if (!titleLabel) {
        titleLabel = new MLabel(this);
        titleLabel->setTextElide(true);
        titleLabel->setStyleName("CommonTitle");
    }
    return titleLabel;
}

MLabel * DocumentListItem::subtitleWidget()
{
    if (!subtitleLabel) {
        subtitleLabel = new MLabel(this);
        subtitleLabel->setTextElide(true);
        subtitleLabel->setStyleName("CommonSubTitle");
        subtitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    }
    return subtitleLabel;
}
MImageWidget *DocumentListItem::imageWidget()
{
    if (!icon) {
        icon = new MImageWidget(this);
        icon->setStyleName("CommonMainIcon");
    }
    return icon;
}

MImageWidget *DocumentListItem::sideTopImageWidget()
{
    if (!sideTopIcon) {
        sideTopIcon = new MImageWidget(this);
        sideTopIcon->setStyleName("CommonSubIconTop");
    }
    return sideTopIcon;
}

MLabel * DocumentListItem::sideBottomSubtitleWidget()
{
    if (!sideBottomSubtitleLabel) {
        sideBottomSubtitleLabel = new MLabel(this);
        sideBottomSubtitleLabel->setTextElide(false);
        sideBottomSubtitleLabel->setStyleName("CommonItemInfo");
        // we use 02 02 2000 as a 1 is smaller with the current used font
//        MLocale  locale;
//        sideBottomSubtitleLabel->setText(locale.formatDateTime(QDateTime::fromString("02/02/2000", "dd/MM/yyyy"),
//                                                               MLocale::DateShort,MLocale::TimeNone));
        sideBottomSubtitleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    }
    return sideBottomSubtitleLabel;
}

MProgressIndicator * DocumentListItem::spinner()
{
    if (!itemSpinner) {
        itemSpinner = new MProgressIndicator(this, MProgressIndicator::spinnerType);
        itemSpinner->setStyleName("CommonListItemSpinner");
        itemSpinner->setVisible(false);
        itemSpinner->reset();
    }
    return itemSpinner;
}

void DocumentListItem::showSpinner()
{
    if (m_page && m_page->showSpinner()) {
        // make sure the spinner exists
        spinner();
        sideBottomSubtitleLabel->setVisible(false);
        layoutPolicy->removeItem(sideBottomSubtitleLabel);
        sideTopIcon->setVisible(false);
        layoutPolicy->removeItem(sideTopIcon);
        itemSpinner->setVisible(true);
        layoutPolicy->addItem(spinner(), 0, 3, 3, 1, Qt::AlignRight | Qt::AlignVCenter);
        itemSpinner->setUnknownDuration(true);
        QTimer::singleShot(5000, this, SLOT(hideSpinner()));
    }
}

void DocumentListItem::hideSpinner()
{
    if (itemSpinner) {
        itemSpinner->setVisible(false);
        itemSpinner->reset();
        layoutPolicy->removeItem(spinner());
        sideBottomSubtitleLabel->setVisible(true);
        layoutPolicy->addItem(sideTopImageWidget(), 0, 3, Qt::AlignRight | Qt::AlignBottom);
        sideTopIcon->setVisible(true);
        layoutPolicy->addItem(sideBottomSubtitleWidget(), 1, 2, 1, 2, Qt::AlignRight | Qt::AlignBottom);
    }
}

void DocumentListItem::setTitle(const QString &text)
{
    //If the title is changed then Recycler is using this item
    //then hide the spinner
    if (QString::compare(text, titleLabel->text()) != 0) {
        hideSpinner();
    }
    QString title = text;
    QString highlightText = m_page ? m_page->highlightText() : QString();
    if (!highlightText.isEmpty()) {
        int pos = 0;
        while (pos != -1) {
            pos = title.indexOf(highlightText, pos, Qt::CaseInsensitive);
            if (pos != -1) {
                title.insert(pos, "<u><a href=\"\">");
                pos = pos + highlightText.count() + 14; //Move to size of link tag
                title.insert(pos, "</a></u>");
                pos += 8; //size of closing tag
            }
        }
    }
    titleLabel->setText(title);
}

void DocumentListItem::setSubtitle(const QString &text)
{
    QString title = text;

    QString highlightText = m_page ? m_page->highlightText() : QString();
    if (!highlightText.isEmpty()) {
        int pos = 0;
        while (pos != -1) {
            pos = title.indexOf(highlightText, pos, Qt::CaseInsensitive);
            if (pos != -1) {
                title.insert(pos, "<u><a href=\"\">");
                pos = pos + highlightText.count() + 14; //Move to size of link tag
                title.insert(pos, "</a></u>");
                pos += 8; //size of closing tag
            }
        }
    }
    subtitleLabel->setText(title);
}

void DocumentListItem::setSideBottomTitle(const QString &text)
{
    Q_UNUSED(text);
//    sideBottomSubtitleLabel->setText(text);
}

void DocumentListItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    MListItem::resizeEvent(event);

    if (!layout)
        setLayout(createLayout());
}

void DocumentListItem::setPage(DocumentListPage *page)
{
    m_page = page;
}
