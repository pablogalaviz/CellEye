//
// Created by Pablo Galaviz on 31/5/21.
//

#include "pointItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QPen>

pointItem::pointItem(int _id, QGraphicsItem *parent) : QGraphicsEllipseItem(parent) { id=_id; }
pointItem::pointItem(int _id, qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) : QGraphicsEllipseItem(x,y,width, height,parent){ id= _id;};
pointItem::pointItem(int _id, const QRectF &rect, QGraphicsItem *parent): QGraphicsEllipseItem(parent){ id=_id;};

void pointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::red));
    emit(setFrame(id));
}

int pointItem::getId() const {
    return id;
}


