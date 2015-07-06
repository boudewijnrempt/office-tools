#ifndef PdfPageWidget_H
#define PdfPageWidget_H

#include <MWidget>
#include <QObject>
#include <poppler-qt4.h>

#include "pdfloader.h"
#include "zoomlevel.h"
#include "basepagewidget.h"
#include "documentviewer_export.h"
namespace Poppler
{

class Page;
}

class PdfLoader;

class MProgressIndicator;
/*!
 * \class PdfPageWidget
 * \brief A widget for showing pdf page.
 */

class DOCUMENTVIEWER_EXPORT PdfPageWidget : public MWidget , public BasePageWidget
{
    Q_OBJECT

signals:
    /*!
     * \brief Signal that is sent when user taps the image.
     * \param pageIndex is the page index (from zero)
     * \param rPoint is the relative page point to be shown
     */
    void showPage(int pageIndex, QPointF rPoint);
    void requestApplicationQuit();
    void requestApplicationClose();
    void requestSearch();
    void changZoomLevel(ZoomLevel level);

public:
    PdfPageWidget(PdfLoader *loader, int pageIndex, MWidget *parent = 0);
    ~PdfPageWidget();

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    void update(const QRectF & rect = QRectF());
    void updateupdate (qreal x, qreal y, qreal width, qreal height);
    /*!
     * \brief Calculate new widget size. Causes resizing of the widget.
     * \param viewSize is the scene visible size needed in 'fit to width'
     * \param zoom is the new zoom level
     */
    void updateSize(const QSizeF & viewSize, const ZoomLevel & zoom);

    /*!
     * \brief Calculate 'poppler' scale
     * \param targetSize The original size
     * \param sourceSize the size where to scale
     * \return How many pixels there are per inch. The 100% scale has value 72
     */
    static qreal calcScale(const qreal targetSize, const qreal sourceSize);

    /*!
     * \brief Calculate size using 'poppler' scale
     * \param scale The 'poppler' scale
     * \param size  The orginal size to be scaled
     * \return The scaled size
     */
    static qreal calcScaledSized(const qreal scale, const qreal size);

    /*!
     * \brief Checks if point in PDF is a link. Handles the link if it is.
     * \param point in the widget
     * \return true if point was a link
     */
    bool linkTaped(const QPointF &point);

#ifdef SELECT_TEXT
    /*!
     * \brief Method for text selection
     * \param startPoint Point where text selection starts
     * \param endPoint Point where text selection ends (must have bigger y then startPoint)
     */
    void selectText(QPointF startPoint, QPointF endPoint);

    /*!
     * \brief Clears current text selection
     */
    void clearSelectedText();

    /*!
     * \brief Clears current text selection and text box list.
     */
    void clearFullSelectedText();
#endif

    /*!
     * \brief Zoomfactor of the page.
     * \return zoom factor
     */
    qreal calcZoomFactor();
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;


protected:
    bool handleLinkTypes(const Poppler::Link *link);
    void handleLinkAction(const Poppler::LinkAction *link);
    void handleLinkGoto(const Poppler::LinkGoto *link);
    void handleLinkDestination(const Poppler::LinkDestination &destination);
#ifdef SELECT_TEXT
    void calcTextBoxList(void);
#endif
    qreal zoomToScale(const QSizeF & viewSize, const ZoomLevel & zoom, const QSize &pageSize);

private slots:
    void clearCachedImage();

private:
    PdfLoader       *loader;
    QSizeF          widgetSize;
    qreal           scale;
    ZoomLevel       lastZoomLevel;
    QSizeF          lastViewSize;
#ifdef SELECT_TEXT
    QList<Poppler::TextBox*> textBoxList;
    QList<QRectF>   selectedRectList;
#endif
    qreal           lastUserDefinedFactor;
    static const qreal maximumScale;
    MProgressIndicator *spinner;
    QPointF         spinnerCenter;
    QImage m_cachedImage;
    bool m_updateCachedImage;
};

#endif //PdfPageWidget_H
