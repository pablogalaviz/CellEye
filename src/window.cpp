//
// Created by pablo on 12/7/20.
//

#include "window.h"
#include "logger.h"
#include "ImageItem.h"
#include "plotDialog.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsView>
#include <boost/property_tree/json_parser.hpp>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QErrorMessage>

#include <QtWidgets/QGraphicsPixmapItem>
#include <QtGui/QVector2D>
#include <QtGui/QBitmap>
#include <QtWidgets/QMessageBox>
#include <random>


Window::Window(QMainWindow *parent, bool _debug) {

    debug = _debug;
    main_ui.setupUi(parent);

    QObject::connect(main_ui.action_Open, SIGNAL (triggered(bool)), this, SLOT (openFile(bool)));
    QObject::connect(main_ui.action_Save, SIGNAL (triggered(bool)), this, SLOT (saveFile(bool)));
    QObject::connect(main_ui.action_Next, SIGNAL (triggered()), this, SLOT (next()));
    QObject::connect(main_ui.action_Play, SIGNAL (triggered()), this, SLOT (auto_process()));
    QObject::connect(main_ui.action_Prev, SIGNAL (triggered()), this, SLOT (previous()));
    QObject::connect(main_ui.action_Plot, SIGNAL (triggered()), this, SLOT (plot()));
    QObject::connect(main_ui.iterationsSpinBox, SIGNAL (valueChanged(int)), this, SLOT (changeIterations(int)));
    QObject::connect(main_ui.thresholdHorizontalSlider, SIGNAL (valueChanged(int)), this, SLOT (changeThreshold(int)));
    QObject::connect(main_ui.imgThresholdHorizontalSlider, SIGNAL (valueChanged(int)), this,
                     SLOT (changeImgThreshold(int)));
    QObject::connect(main_ui.errorHorizontalSlider, SIGNAL (valueChanged(int)), this, SLOT (changeError(int)));
    QObject::connect(main_ui.experimentNameLineEdit, SIGNAL (textEdited(QString)), this, SLOT (changeName(QString)));
    QObject::connect(main_ui.experimentIdSpinBox, SIGNAL (valueChanged(int)), this, SLOT (changeId(int)));
    QObject::connect(main_ui.processPushButton, SIGNAL (toggled(bool)), this, SLOT (changeProcessState(bool)));
    QObject::connect(main_ui.flagPushButton, SIGNAL (toggled(bool)), this, SLOT (changeFlagState(bool)));

    QObject::connect(this, SIGNAL (updateState(bool)), main_ui.processPushButton, SLOT (setChecked(bool)));
    QObject::connect(this, SIGNAL (updateStopState(bool)), main_ui.processStopPushButton, SLOT (setChecked(bool)));
    QObject::connect(this, SIGNAL (updateId(int)), main_ui.experimentIdSpinBox, SLOT (setValue(int)));
    QObject::connect(this, SIGNAL (updateName(QString)), main_ui.experimentNameLineEdit, SLOT (setText(QString)));
    QObject::connect(this, SIGNAL (updateArea(double)), main_ui.AreaLcdNumber, SLOT (display(double)));
    QObject::connect(this, SIGNAL (updateError(double)), main_ui.ErrorLcdNumber, SLOT (display(double)));
    QObject::connect(this, SIGNAL (updateIterations(int)), main_ui.iterationsSpinBox, SLOT (setValue(int)));
    QObject::connect(this, SIGNAL (updateThreshold(int)), main_ui.thresholdHorizontalSlider, SLOT (setValue(int)));
    QObject::connect(this, SIGNAL (updateImgThreshold(int)), main_ui.imgThresholdHorizontalSlider,
                     SLOT (setValue(int)));
    QObject::connect(this, SIGNAL (updateErrorRegion(int)), main_ui.errorHorizontalSlider, SLOT (setValue(int)));

    //QObject::connect(this, SIGNAL (lockProcessing( bool )), main_ui.iterationsSpinBox, SLOT (setEnabled( int)));


}


QImage Mat2QImage(cv::Mat const &src) {
    cv::Mat temp; // make the same cv::Mat
    cvtColor(src, temp, cv::COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits(); // enforce deep copy, see documentation
    // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return dest;
}

cv::Mat QImage2Mat(QImage const &src) {
    cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar *) src.bits(), src.bytesPerLine());
    cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
    cvtColor(tmp, result, cv::COLOR_BGR2RGB);
    return result;
}

void Window::loadData() {

    emit updateName(QString::fromStdString(session_data.name));
    emit updateId(session_data.id);

}

void Window::loadFrame() {

    if (session_data.data.find(current) != session_data.data.end()) {
        currentFrameData = session_data.data[current];
        processState = false;
    } else {
        //
        // session_data.data[current]=currentFrameData;
        processState = true;
    }

    emit updateIterations(currentFrameData.iterations);
    emit updateThreshold(static_cast<int>(std::round(currentFrameData.threshold * 100)));
    emit updateImgThreshold(currentFrameData.img_threshold);
    emit updateErrorRegion(currentFrameData.error);
    emit updateArea(currentFrameData.area_path);
    emit updateError(currentFrameData.error_area_path);
    emit updateState(processState);
    emit updateStopState(!processState);


}

void Window::saveData() {

    std::ofstream file(dataFileName);
    if (processState) {
        session_data.data[current] = currentFrameData;
    }
    file << serialize(session_data);
    file.close();

}

void Window::saveFile(bool checked) {


    if (images.empty() || session_data.data.empty()) {
        QErrorMessage errorMessage(this);
        errorMessage.showMessage("Nothing to save\n");
        errorMessage.exec();
    } else {

        boost::filesystem::path default_file = boost::filesystem::change_extension(session_data.file_name, ".csv");
        QString qt_fileName = QFileDialog::getSaveFileName(this, "Save Data",
                                                           QString::fromStdString(default_file.string()),
                                                           "Csv Files (*.csv)");
        if (qt_fileName.isEmpty()) { return; }
        std::string fileName = qt_fileName.toStdString();
        boost::filesystem::path data_file = boost::filesystem::change_extension(fileName, ".csv");

        LOGGER.info << "Save to file: " << data_file << std::endl;


        std::ofstream file(data_file.string());

        file << "name"
             << ",id"
             << ",frame"
             << ",area_path"
             << ",error_area_path"
             << ",area_pixel"
             << ",error_area_pixel"
             << ",flag"
             << std::endl;


        for (auto &item : session_data.data) {

            file << session_data.name
                 << "," << session_data.id
                 << "," << item.first + 1
                 << "," << item.second.area_path
                 << "," << item.second.error_area_path
                 << "," << item.second.area_pixel
                 << "," << item.second.error_area_pixel
                 << "," << (item.second.flag ? "True" : "False")
                 << std::endl;
        }

        file.close();

    }

}

void Window::openFile(bool checked) {

    QString qt_fileName = QFileDialog::getOpenFileName(this, "Open Image", QDir::homePath(),
                                                       "Image Files (*.tiff *tif)");
    if (qt_fileName.isEmpty()) { return; }

    std::string fileName = qt_fileName.toStdString();

    LOGGER.info << "Opening file: " << fileName << std::endl;

    images.clear();
    pathItems.clear();

    if (cv::imreadmulti(fileName, images, cv::IMREAD_COLOR)) {

        boost::filesystem::path data_file = boost::filesystem::change_extension(fileName, ".json");
        dataFileName = data_file.string();
        LOGGER.debug << "data_file " << data_file << std::endl;
        if (boost::filesystem::exists(data_file)) {
            boost::property_tree::ptree data_tree;
            boost::property_tree::read_json(dataFileName, data_tree);
            session_data.deserialize(data_tree);
            processState = false;
        } else {
            session_data.name = boost::filesystem::basename(fileName);
            session_data.file_name = fileName;
            session_data.id = 1;
            processState = true;
        }

        loadData();
        current = 0;
        main_ui.progressBar->setMinimum(0);
        main_ui.progressBar->setMaximum(images.size());
        if (processState) {
            currentFrameData.roi = cv::Rect(0, 0, images[0].cols, images[0].rows);
        }
        update(true);
        loadFrame();
        update(false);

    } else {
        LOGGER.error << "Unable to open file: " << fileName << std::endl;
        QErrorMessage errorMessage(this);
        errorMessage.showMessage("Unable to open file:\n" + qt_fileName);
        errorMessage.exec();
    }

}

void Window::auto_process() {

    if (processing || images.empty()) {
        return;
    }
    processing = true;

    LOGGER.debug << "Auto process " << std::endl;

    QProgressDialog progress("Task in progress...", "Cancel", 0, images.size(), this);
    progress.setWindowModality(Qt::WindowModal);

    frameData previous = currentFrameData;
    std::vector<frameData> processResult;

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dist_1_100(1, 100);
    std::uniform_int_distribution<> dist_1_255(1, 255);


    int max_iter = 1000;
    double error_tolerance = 0.01;
    for (int i = current + 1; i < images.size(); i++) {
        progress.setValue(i);

        cv::Mat to_proc = images[i](previous.roi);
        frameData bestFit;
        frameData data = previous;

        int count;
        for (count = 0; count < max_iter; count++) {

            process(to_proc, data);
            data.diff = abs(data.area_pixel - previous.area_pixel) / previous.area_pixel;

            if(data.diff < bestFit.diff){
                bestFit=data;
            }

            if (data.diff < error_tolerance) { break; }


            data.threshold = rand() / RAND_MAX;
            data.iterations = dist_1_100(gen);
            data.img_threshold = dist_1_255(gen);
        }

        if(count > 0) {
            LOGGER.warning << "iterations " << count << std::endl;
        }

        LOGGER.debug << "Frame " << i << " diff: " << bestFit.diff << std::endl;

        processResult.push_back(bestFit);
        previous = bestFit;

        if (progress.wasCanceled())
            break;
    }

    std::ofstream file(dataFileName);
    for(int i = current; i < current+processResult.size(); i++ ){
        session_data.data[i] = processResult[i];
    }
    file << serialize(session_data);
    file.close();

    progress.setValue(images.size());

    LOGGER.debug << "Done " << std::endl;

    cv::destroyAllWindows();

    processing = false;



}



void  Window::plot(){

   //if(session_data.data.empty()){return;}

    QDialog dialog;

    plotDialog plot_ui(&dialog,session_data.data);

    dialog.exec();

}

void Window::update(bool reload) {

    if (images.empty()) { return; }

    main_ui.progressBar->setValue(current + 1);

    LOGGER.debug << " frame " << current << std::endl;


    //START PROCESS

    cv::Mat to_proc = images[current](currentFrameData.roi);

    if (processState) {
        process(to_proc, currentFrameData);
        emit updateArea(currentFrameData.area_path);
        emit updateError(currentFrameData.error_area_path);
    }
    if (reload) {
        QGraphicsScene *scene = new QGraphicsScene();

        QImage q_image = Mat2QImage(images[current]);
        ImageItem *item = new ImageItem(QPixmap::fromImage(q_image));
        item->setAcceptedMouseButtons(Qt::LeftButton);
        item->setAcceptHoverEvents(true);

        QObject::connect(item, SIGNAL(setCenter(double, double)), this, SLOT (setCenter(double, double)));
        QObject::connect(item, SIGNAL(moveCenter(double, double)), this, SLOT (moveCenter(double, double)));
        QObject::connect(item, SIGNAL(unsetCenter(double, double)), this, SLOT (unsetCenter(double, double)));

        scene->addItem(item);

        LOGGER.debug << "items:" << scene->items().count() << std::endl;

        if (main_ui.graphicsView->scene() != nullptr) {
            main_ui.graphicsView->scene()->clear();
        }
        main_ui.graphicsView->setScene(scene);

    } else {
        for (auto &path : pathItems) {
            main_ui.graphicsView->scene()->removeItem(path);
        }
        pathItems.clear();
    }

    QPen pen(Qt::green, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    roiRectangle = main_ui.graphicsView->scene()->addRect(currentFrameData.roi.x, currentFrameData.roi.y,
                                                          currentFrameData.roi.width, currentFrameData.roi.height, pen);

    pathItems.clear();
    QPen path_pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QColor errorColor(255, 55, 55, 125);
    QPen error_pen(errorColor, currentFrameData.error * 2 + 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    for (auto &item : currentFrameData.contours) {
        QPainterPath new_path(QPointF(currentFrameData.roi.x + item[0].x, currentFrameData.roi.y + item[0].y));
        for (int i = 1; i < item.size(); i++) {
            new_path.lineTo(currentFrameData.roi.x + item[i].x, currentFrameData.roi.y + item[i].y);
        }
        pathItems.push_back(main_ui.graphicsView->scene()->addPath(new_path, error_pen));
        pathItems.push_back(main_ui.graphicsView->scene()->addPath(new_path, path_pen));
    }


}


void Window::setCenter(double x, double y) {

    if (roiRectangle != nullptr && processState) {
        //main_ui.graphicsView->scene()->removeItem(roiRectangle);

        new_roi.x = x;
        new_roi.y = y;
        new_roi.width = 1;
        new_roi.height = 1;

        QPen pen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        main_ui.graphicsView->scene()->removeItem(roiRectangle);
        roiRectangle = main_ui.graphicsView->scene()->addRect(new_roi.x, new_roi.y, new_roi.width, new_roi.height, pen);
    }
}

void Window::moveCenter(double x, double y) {

    if (roiRectangle != nullptr && processState) {
        main_ui.graphicsView->scene()->removeItem(roiRectangle);

        new_roi.width = abs(new_roi.x - x);
        new_roi.height = abs(new_roi.y - y);

        new_roi.x = std::min(static_cast<int>(std::round(x)), new_roi.x);
        new_roi.y = std::min(static_cast<int>(std::round(y)), new_roi.y);

        QPen pen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        roiRectangle = main_ui.graphicsView->scene()->addRect(new_roi.x, new_roi.y, new_roi.width, new_roi.height, pen);
    }

}

void Window::unsetCenter(double x, double y) {

    if (!processState) { return; }

    new_roi.width = abs(new_roi.x - x);
    new_roi.height = abs(new_roi.y - y);

    new_roi.x = std::min(static_cast<int>(std::round(x)), new_roi.x);
    new_roi.y = std::min(static_cast<int>(std::round(y)), new_roi.y);

    if (new_roi.width != 0 && new_roi.height != 0) { currentFrameData.roi = new_roi; }

    update(true);

    /*
    cv::Mat to_proc = images[current](roi);
    std::vector<std::vector<cv::Point> > contours = process(to_proc);

    for(auto &path : pathItems){
        main_ui.graphicsView->scene()->removeItem(path);
    }
    pathItems.clear();
    QPen path_pen(Qt::darkRed, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    for(auto &item : contours){
        QPainterPath new_path(QPointF(roi.x+item[0].x,roi.y+item[0].y));
        for(int i=1; i < item.size(); i++){
            new_path.lineTo(roi.x+item[i].x,roi.y+item[i].y);
        }
        pathItems.push_back(main_ui.graphicsView->scene()->addPath(new_path,path_pen));
    }
*/

}

void Window::process(const cv::Mat &image, frameData &data) {

    LOGGER.debug << "iterations: " << data.iterations << std::endl;
    LOGGER.debug << "threshold: " << data.threshold << std::endl;
    LOGGER.debug << "img_threshold: " << data.img_threshold << std::endl;

    std::vector<std::vector<cv::Point> > contours;

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    if (debug) {
        cv::imshow("gray", gray);
    }
    cv::Mat thresh;
//    cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    int th_value = cv::threshold(gray, thresh, data.img_threshold, 255, cv::THRESH_BINARY_INV);
    LOGGER.debug << "th_value: " << th_value << std::endl;


    if (debug) {
        cv::imshow("thresh", thresh);
    }

    cv::Mat opening;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(thresh, opening, cv::MORPH_OPEN, element, cv::Point(-1, -1), data.iterations);

    if (debug) {
        cv::imshow("opening", opening);
    }

    cv::Mat sure_bg;
    cv::morphologyEx(opening, sure_bg, cv::MORPH_DILATE, element, cv::Point(-1, -1), data.iterations);

    if (debug) {
        cv::imshow("sure_bg", sure_bg);
    }

    cv::Mat dist_transform;
    cv::distanceTransform(opening, dist_transform, cv::DIST_L2, 5);

    if (debug) {
        cv::imshow("dist_transform", dist_transform);
    }

    cv::Mat sure_fg;
    double min_val, max_val;
    cv::minMaxIdx(dist_transform, &min_val, &max_val);
    cv::threshold(dist_transform, sure_fg, data.threshold * max_val, 255, CV_8U);
    sure_fg.convertTo(sure_fg, CV_8U);

    if (debug) {
        cv::imshow("sure_fg", sure_fg);
    }

    cv::Mat unknown;
    cv::subtract(sure_bg, sure_fg, unknown);

    cv::Mat markers;
    cv::connectedComponents(sure_fg, markers);


    cv::Mat markers_32;
    markers.convertTo(markers_32, CV_32SC1);
    markers_32 += 1;
    markers_32.setTo(cv::Scalar(0, 0, 0), unknown == 255);


    //markers=markers_32*127;
    //cv::minMaxIdx(markers,&min_val,&max_val);
    //LOGGER.debug << " mask " << min_val << " " << max_val << std::endl;
    //markers.convertTo(markers,CV_8UC3);
    //cv::imshow("markers_32 ",markers);

    cv::Mat image_8uc3;
    image.convertTo(image_8uc3, CV_8UC3);

    cv::watershed(image_8uc3, markers_32);

    cv::minMaxIdx(markers_32, &min_val, &max_val);
    LOGGER.debug << " mask " << min_val << " " << max_val << std::endl;


    //image_8uc3.setTo(cv::Scalar(0, 0, 255), markers_32 == 2);
    //cv::imshow("result ", image_8uc3);

    cv::Mat mask(image_8uc3.rows, image_8uc3.cols, CV_8UC1);
    mask = 0;
    mask.setTo(255, markers_32 == 2);
    mask.setTo(255, markers_32 >= 2);

    if (debug) {
        cv::imshow("mask", mask);
    }


    findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
/*
    for( size_t i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar( 0, 0, 256 );
        drawContours( drawing, contours, (int)i, color, 2, cv::LINE_8 );
    }
    cv::imshow( "Contours", drawing );

cv::imshow("result mask ",mask);

*/

    data.area_pixel = 0;
    for (int j = 0; j < markers_32.rows; j++)
        for (int i = 0; i < markers_32.cols; i++)
            if (markers_32.at<uint>(j, i) == 2)
                data.area_pixel += 1;

    {
        double l = sqrt(data.area_pixel);
        double Ap = pow(l + data.error, 2);
        double Am = pow(l - data.error, 2);
        data.error_area_pixel = Ap - Am;
    }

    data.area_path = 0;
    for (auto &c : contours) {
        LOGGER.debug << "contour size: " << c.size() << std::endl;
        data.area_path += cv::contourArea(c);
    }

    LOGGER.debug << "Area pixel: " << data.area_pixel << " area_path " << data.area_path
                 << std::endl;

    {
        double l = sqrt(data.area_path);
        double Ap = pow(l + data.error, 2);
        double Am = pow(l - data.error, 2);
        data.error_area_path = Ap - Am;
    }

    data.contours = contours;

}





