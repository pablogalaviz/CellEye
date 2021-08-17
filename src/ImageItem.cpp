//
// Created by pablo on 5/8/20.
//

#include "ImageItem.h"
#include "logger.h"
#include <QGraphicsSceneMouseEvent>

ImageItem::ImageItem(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {}

ImageItem::ImageItem(const QPixmap &pixmap, QGraphicsItem *parent) : QGraphicsPixmapItem(pixmap, parent) {}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    emit(setPointer(event->pos().x(), event->pos().y()));

}


void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit(movePointer(event->pos().x(), event->pos().y()));
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    emit(unsetPointer(event->pos().x(), event->pos().y()));
}

