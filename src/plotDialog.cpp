//
// Created by Pablo Galaviz on 27/5/21.
//

#include <QGraphicsTextItem>
#include "plotDialog.h"


plotDialog::plotDialog(QDialog *parent, std::map<int,frameData> data, int current,Window *window, bool _debug) {

    debug = _debug;
    plot_ui.setupUi(parent);

    parent->setFixedSize(parent->width(),parent->height());

    int high = 360;
    int h_margin = 20;

    std::vector<int> x_data;
    double area_max=0;
    double area_min=1e6;
    for(auto &item : data){
        x_data.push_back(item.first);
        area_max=std::max(area_max,item.second.area_path);
        area_min=std::min(area_min,item.second.area_path);
    }

    double factor=(high-h_margin)/(area_max-area_min);

    std::vector<int> y_data;
    for(auto &item : data) {
        y_data.push_back(int(floor(h_margin*0.5+factor*(item.second.area_path-area_min))));
    }


    QGraphicsScene *scene = new QGraphicsScene();

    scene->setBackgroundBrush(Qt::white);

    plot_ui.graphicsView->setScene(scene);

    int x_step = 30;
    int width = std::max(200ul,x_step*data.size());

    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    plot_ui.graphicsView->scene()->addRect(0
            , 0
            , width
            , high
            , pen);

    int y_origin = high;
    int tick_size = 5;

     int font_size=12;
    QFont serifFont("Times", font_size, QFont::Bold);

    for(int i=0; i < data.size(); i++) {
        plot_ui.graphicsView->scene()->addLine(i*x_step, y_origin, i*x_step, y_origin+tick_size, pen);
        QGraphicsTextItem *label = plot_ui.graphicsView->scene()->addText(std::to_string(i).c_str(),serifFont);
        label->setPos(i*x_step-font_size/2,y_origin+tick_size);
        label->setDefaultTextColor(Qt::black);

        pointItem *item = new pointItem(x_data[i], x_data[i]*x_step,high-y_data[i],5,5);
        if(x_data[i] == current){
            item->setPen(QPen(Qt::red));
            item->setBrush(QBrush(Qt::red));
        }
        else {
            item->setPen(QPen(Qt::blue));
            item->setBrush(QBrush(Qt::blue));
        }
        item->setAcceptedMouseButtons(Qt::LeftButton);
        points.push_back(item);
        QObject::connect(item, SIGNAL(setFrame(int)), window, SLOT (setFrame(int)));
        QObject::connect(item, SIGNAL(setFrame(int)), this, SLOT (setFrame(int)));

        plot_ui.graphicsView->scene()->addItem(item);

    }

    int precision=5;
    int y_step = high/4;
    int x_origin = 0.0;
    for(int j=0; j <= high; j+=y_step) {
        plot_ui.graphicsView->scene()->addLine(x_origin, y_origin-j, x_origin-tick_size, y_origin-j, pen);
        double area=(j-0.5*h_margin)/factor+area_min;
        std::stringstream ss;
        ss << std::setprecision(precision) << area;
        QGraphicsTextItem *label = plot_ui.graphicsView->scene()->addText(ss.str().c_str(),serifFont);

        label->setPos(x_origin-tick_size-font_size*precision,y_origin-j-font_size);
        label->setDefaultTextColor(Qt::black);

    }

}


void plotDialog::setFrame(int id){

    for(auto  &item : points){
        if(item->getId() == id){continue;}
        item->setBrush(QBrush(Qt::blue));
        item->setPen(QPen(Qt::blue));
    }


}