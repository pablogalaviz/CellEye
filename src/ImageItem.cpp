//
// Created by pablo on 5/8/20.
//

#include "ImageItem.h"
#include "logger.h"
#include <QGraphicsSceneMouseEvent>

ImageItem::ImageItem(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {}

ImageItem::ImageItem(const QPixmap &pixmap, QGraphicsItem *parent) : QGraphicsPixmapItem(pixmap, parent) {}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    emit(setCenter(event->pos().x(), event->pos().y() ));

}


void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit(moveCenter(event->pos().x(), event->pos().y()));
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    emit(unsetCenter(event->pos().x(), event->pos().y()));
}

