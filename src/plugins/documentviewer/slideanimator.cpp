#include "slideanimator.h"
#include "definitions.h"
#include "applicationwindow.h"

#include <MImageWidget>
#include <MApplication>
#include <MPannableViewport>
#include <MPositionIndicator>

#include <QPropertyAnimation>
#include <QGraphicsLinearLayout>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

const qreal SlideAnimator::animationGap = 10.0;
static const int ANIMATION_DURATION = 400;
static const int STATUSBARSIZE = 0;

SlideAnimator::SlideAnimator(QGraphicsItem *parent)
    : MWidgetController(parent)
    , m_animation(0)
    , m_image(new MImageWidget())
    , m_cancelImage(0)
    , m_panAnimation(0)
    , m_viewport(0)
{
    m_animation = new QPropertyAnimation(this, "paintOffset");

    QGraphicsLinearLayout * layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->addItem(m_image);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // show below the page indicator
    setZValue(ZValuePageIndicator - 1);
    setVisible(false);
}

SlideAnimator::~SlideAnimator()
{
    delete m_animation;
    delete m_panAnimation;
}

void SlideAnimator::slide(MPannableViewport *viewport, Direction direction)
{
    if (m_animation->state() == QAbstractAnimation::Running) {
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << paintOffset().y() << geometry();

    m_viewport = viewport;
    delete m_panAnimation;
    m_panAnimation = new QPropertyAnimation(viewport, "paintOffset");
    connect(m_panAnimation, SIGNAL(finished()), this, SLOT(slotAnimationCompleted()));
    m_direction = direction;

    m_panAnimation->setStartValue(QPointF(0, 0));
    m_panAnimation->setEndValue(QPointF(0, -paintOffset().y() + STATUSBARSIZE));
    m_panAnimation->setDuration(ANIMATION_DURATION);
    m_panAnimation->setEasingCurve(QEasingCurve::OutQuint);
    m_panAnimation->start();

    m_animation->setStartValue(QPointF(0, paintOffset().y()));
    m_animation->setEndValue(QPointF(0, STATUSBARSIZE));
    m_animation->setDuration(ANIMATION_DURATION);
    m_animation->setEasingCurve(QEasingCurve::OutQuint);
    m_animation->start();
}

void SlideAnimator::slideCancel(MPannableViewport *viewport, const QPixmap &pixmap, const QPointF &offset, Direction direction)
{
    if (m_animation->state() == QAbstractAnimation::Running) {
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << paintOffset().y() << geometry() << offset << viewport->position();

    m_viewport = viewport;
    delete m_panAnimation;

    m_panAnimation = new QPropertyAnimation(viewport, "position");
    connect(m_panAnimation, SIGNAL(finished()), this, SLOT(slotAnimationCancelCompleted()));
    m_direction = direction;

    // we animate the position of the pannable scrollbars but we don't show the
    // actual document but a image containing its content to get a good fps
    // the image is painted with an offset.
    // and as the offset of the parent is manipulated it works quite nicely.
    viewport->setVisible(false);
    m_cancelImage = new MImageWidget(this);
    m_cancelImage->setPixmap(pixmap);
    m_cancelImage->setMinimumSize(pixmap.size().width(), pixmap.size().height());
    if (direction == Previous) {
        m_cancelImage->setPaintOffset(QPointF(offset.x(), offset.y() + m_image->size().height() + animationGap));
        m_panAnimation->setEndValue(QPointF(viewport->position().x(), 0));
    }
    else {
        qreal height = ApplicationWindow::visibleSize().height();
        m_cancelImage->setPaintOffset(QPointF(offset.x(), offset.y() - m_cancelImage->size().height() - animationGap));
        m_panAnimation->setEndValue(QPointF(viewport->position().x(), m_cancelImage->size().height() <= height ? 0: m_cancelImage->size().height() - height));
    }

    m_panAnimation->setStartValue(viewport->position());
    m_panAnimation->setDuration(ANIMATION_DURATION);
    m_panAnimation->start();
}

void SlideAnimator::setPixmap(const QPixmap &pixmap)
{
    m_image->setPixmap(pixmap);
    m_image->setMinimumSize(pixmap.size().width(), pixmap.size().height());
    setVisible(true);
}

void SlideAnimator::setDirection(Direction direction)
{
    m_direction = direction;
}

void SlideAnimator::updatePaintOffset(const QPointF &point, const QPointF &offset)
{
    QSize size = ApplicationWindow::visibleSize();
    if (m_direction == Next) {
        setPaintOffset(QPointF(0, size.height() - point.y() + offset.y() + STATUSBARSIZE + animationGap)); // the STATUSBARSIZE is needed for the status bar
    }
    else {
        setPaintOffset(QPointF(0, -size.height() - point.y() + offset.y() + STATUSBARSIZE - animationGap)); // the STATUSBARSIZE is needed for the status bar
    }
}

void SlideAnimator::slotAnimationCompleted()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_viewport->setPaintOffset(QPointF(0, 0));
    m_viewport->setPosition(QPointF(0, 0));
    if (m_direction == Next) {
        emit animationNextFinished();
    }
    else {
        emit animationPreviousFinished();
    }
    setVisible(false);
    setPaintOffset(QPointF(0, 0));
    m_viewport->physics()->setEnabled(true);
    m_viewport->positionIndicator()->setEnabled(true);
}

void SlideAnimator::slotAnimationCancelCompleted()
{
    delete m_cancelImage;
    qDebug() << __PRETTY_FUNCTION__;
    setVisible(false);
    setPaintOffset(QPointF(0, 0));
    m_viewport->setVisible(true);
    m_viewport->physics()->setEnabled(true);
    m_viewport->positionIndicator()->setEnabled(true);
    emit animationCanceled();
}
