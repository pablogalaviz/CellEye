//
// Created by Pablo Galaviz on 27/5/21.
//

#ifndef CELLEYE_PLOTDIALOG_H
#define CELLEYE_PLOTDIALOG_H


#include <QDialog>
#include <qt/ui_plot.h>
#include "frameData.h"
#include "cellEye.h"
#include "pointItem.h"


class plotDialog : public QObject
{
    Q_OBJECT

    std::vector<pointItem*> points;
    std::vector<QGraphicsLineItem*> error_lines;

    bool debug;
    Ui::Dialog plot_ui;

public:
    explicit plotDialog(QDialog *parent = 0, std::map<int,frameData> data = std::map<int,frameData>(), int current=0, CellEyeWindow *window = 0, bool _debug = false);

protected slots:

    void setFrame(int id);


};


#endif //CELLEYE_PLOTDIALOG_H
