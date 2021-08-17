//
// Created by pablo on 5/8/20.
//

#ifndef CELLEYE_IMAGEITEM_H
#define CELLEYE_IMAGEITEM_H

#include <QWidget>
#include <QtWidgets/QGraphicsPixmapItem>

class ImageItem : public QObject , public QGraphicsPixmapItem{

    Q_OBJECT

public:
    explicit ImageItem(QGraphicsItem *parent = nullptr);
    explicit ImageItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

    signals:
    void setPointer(double x, double y);
    void movePointer(double x, double y);
    void unsetPointer(double x, double y);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:

    double xc,yc;


};


#endif //CELLEYE_IMAGEITEM_H
