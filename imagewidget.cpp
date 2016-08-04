#include <QtDebug>
#include "imagewidget.h"

#include <QPainter>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent), img(":/glass.png")
{

}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto scaled = img.scaled(size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    painter.drawImage(
        width()  / 2 + scaled.width()  * (offset.x() - 0.5),
        height() / 2 + scaled.height() * (offset.y() - 0.5),
        scaled
    );
}

void ImageWidget::addOffset(qreal x, qreal y)
{
    this->offset = QPointF(x,y);
}

