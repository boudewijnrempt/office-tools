#ifndef SLIDEANIMATOR_H
#define SLIDEANIMATOR_H

#include <MWidgetController>

class QPropertyAnimation;
class MImageWidget;
class MPannableViewport;

#include <common_export.h>

class COMMON_EXPORT SlideAnimator : public MWidgetController
{
    Q_OBJECT
public:
    enum Direction {
        Previous,
        Next
    };

    explicit SlideAnimator(QGraphicsItem *parent = 0);
    virtual ~SlideAnimator();

    void slide(MPannableViewport *viewport, Direction direction);
    void slideCancel(MPannableViewport *viewport, const QPixmap &pixmap, const QPointF &offset, Direction direction);

    void setPixmap(const QPixmap& pixmap);
    void setDirection(Direction direction);

    void updatePaintOffset(const QPointF &point, const QPointF &offset);

    static const qreal animationGap;

signals:
    void animationPreviousFinished();
    void animationNextFinished();
    void animationCanceled();

private slots:
    void slotAnimationCompleted();
    void slotAnimationCancelCompleted();

private:
    QPropertyAnimation *m_animation;
    MImageWidget *m_image;
    MImageWidget *m_cancelImage;
    QPropertyAnimation *m_panAnimation;
    MPannableViewport *m_viewport;
    Direction m_direction;
};

#endif // SLIDEANIMATOR_H
