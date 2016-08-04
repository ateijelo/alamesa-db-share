#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QImage>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event) override;

    void addOffset(qreal x, qreal y);

signals:

public slots:

private:
    QPointF offset;
    QImage img;
};

#endif // IMAGEWIDGET_H
