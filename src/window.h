//
// Created by pablo on 12/7/20.
//

#ifndef CELLEYE_WINDOW_H
#define CELLEYE_WINDOW_H

#include <QWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QMainWindow>
#include <qt/ui_main.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "sessionData.h"
#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>
#include <QProgressDialog>




class Window : public QWidget
{
Q_OBJECT

    Ui::MainWindow main_ui;
    std::vector<cv::Mat> images;
    int current;
    sessionData session_data;
    std::string dataFileName;
    bool processState;
    bool processing = false;

    frameData currentFrameData;

    bool debug;



public:
    explicit Window(QMainWindow *parent = 0, bool _debug = false);
protected slots:
    void openFile(bool checked);
    void saveFile(bool checked);
    void changeName(QString value){
        session_data.name = value.toStdString();
    }
    void changeId(int value){
        session_data.id = value;
    }
    void changeProcessState(bool value){
        processState = value;
        if(!value){
            saveData(true);
        }
        LOGGER.debug << "process state: " << (value? "true":"false") << std::endl;
    }
    void changeFlagState(bool value){
        currentFrameData.flag = value;
        LOGGER.debug << "flag state: " << (value? "true":"false") << std::endl;
    }
    void changeIterations(int value){
        currentFrameData.iterations=value;
        update(false);
    }
    void changeThreshold(int value){
        currentFrameData.threshold=value/100.0;
        update(false);
    }
    void changeImgThreshold(int value){
        currentFrameData.img_threshold=value;
        update(false);
    }
    void changeError(int value){
        currentFrameData.error=value;
        update(false);
    }
    void setCenter(double x, double y);
    void moveCenter(double x, double y);
    void unsetCenter(double x, double y);
    void setFrame(int id);

    void update(bool reload);
    void next(){
        saveData(processState);
        if(images.empty()){return;}
        if (current < images.size()-1) { current++; }
        loadFrame();
        update(true);
    }
    void previous(){
        saveData(processState);
        if(images.empty()){return;}
        if (current > 0) { current--; }
        loadFrame();
        update(true);
    }


    void auto_process();

    void plot();

signals:
    void updateArea(double value);
    void updateError(double value);
    void updateName(QString value);
    void updateId(int value);
    void updateState(bool value);
    void updateStopState(bool value);
    void updateIterations(int value);
    void updateThreshold(int value);
    void updateImgThreshold(int value);
    void updateErrorRegion(int value);
    void lockProcessing(bool);

private:
    cv::Rect new_roi;
    QGraphicsEllipseItem *ellipseItem;
    QGraphicsRectItem *roiRectangle;
    void process(const cv::Mat &image,frameData &data);
    std::vector<QGraphicsPathItem *> pathItems;

    void loadData();
    void saveData(bool update_session);
    void loadFrame();

};


#endif //CELLEYE_WINDOW_H
