//
// Created by Pablo Galaviz on 31/5/21.
//

#ifndef CELLEYE_POINTITEM_H
#define CELLEYE_POINTITEM_H


#include <QObject>
#include <QGraphicsEllipseItem>

class pointItem  : public QObject , public QGraphicsEllipseItem{

    int id;
public:
    int getId() const;


Q_OBJECT

    public:

    explicit pointItem(int id, QGraphicsItem *parent = nullptr);
    explicit pointItem(int id, qreal x, qreal y, qreal width, qreal height,QGraphicsItem *parent = nullptr);
    explicit pointItem(int id, const QRectF &rect, QGraphicsItem *parent = nullptr);


        signals:
    void setFrame(int frame);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;



};


#endif //CELLEYE_POINTITEM_H
