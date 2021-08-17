//
// Created by pablo on 12/7/20.
//

#include "cellEye.h"
#include "logger.h"
#include "ImageItem.h"
#include "plotDialog.h"
#include "preferencesDialog.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsView>
#include <boost/property_tree/json_parser.hpp>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QErrorMessage>

#include <opencv2/ximgproc/segmentation.hpp>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtGui/QVector2D>
#include <QtGui/QBitmap>
#include <QtWidgets/QMessageBox>
#include <random>

CellEyeWindow::CellEyeWindow(QMainWindow *parent, bool _debug, std::string _preferences_file) {

    preferences_file=_preferences_file;
    processing_debug = _debug;
    main_ui.setupUi(parent);

    QObject::connect(main_ui.action_Open, SIGNAL (triggered(bool)), this, SLOT (openFile(bool)));
    QObject::connect(main_ui.action_Save, SIGNAL (triggered(bool)), this, SLOT (saveFile(bool)));
    QObject::connect(main_ui.action_Next, SIGNAL (triggered()), this, SLOT (next()));
    QObject::connect(main_ui.action_Play, SIGNAL (triggered()), this, SLOT (auto_process()));
    QObject::connect(main_ui.action_Prev, SIGNAL (triggered()), this, SLOT (previous()));
    QObject::connect(main_ui.action_Plot, SIGNAL (triggered()), this, SLOT (plot()));
    QObject::connect(main_ui.action_Preferences, SIGNAL (triggered()), this, SLOT (preferences()));
    QObject::connect(main_ui.RemoveNoiseIterationsSpinBox, SIGNAL (valueChanged(int)), this, SLOT (
            changeRemoveNoiseIterations(int)));
    QObject::connect(main_ui.ImageBinaryThresholdHorizontalSlider, SIGNAL (valueChanged(int)), this, SLOT (changeImageBinaryThreshold(int)));
    QObject::connect(main_ui.checkBoxImageBinaryThreshold, SIGNAL (stateChanged(int)), this, SLOT (activateBinaryThreshold(int)));

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
    QObject::connect(this, SIGNAL (updateIterations(int)), main_ui.RemoveNoiseIterationsSpinBox, SLOT (setValue(int)));
    QObject::connect(this, SIGNAL (updateThreshold(int)), main_ui.ImageBinaryThresholdHorizontalSlider, SLOT (setValue(int)));


    loadPreferences();

}

void CellEyeWindow::loadPreferences(){

    if (boost::filesystem::exists(preferences_file)) {
        LOGGER.debug << "reading preference file: " << preferences_file <<std::endl;
        boost::property_tree::ptree data_tree;
        boost::property_tree::read_json(preferences_file, data_tree);
        preferenceData preference_data;
        preference_data.deserialize(data_tree);
        main_ui.RemoveNoiseIterationsSpinBox->setMaximum(preference_data.maximum_method_iterations);
    }


}


QImage Mat2QImage(cv::Mat const &src) {
    cv::Mat temp; // make the same cv::Mat
    cvtColor(src, temp, cv::COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits(); // enforce deep copy, see documentation
    return dest;
}

cv::Mat QImage2Mat(QImage const &src) {
    cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar *) src.bits(), src.bytesPerLine());
    cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
    cvtColor(tmp, result, cv::COLOR_BGR2RGB);
    return result;
}

void CellEyeWindow::loadData() {

    emit updateName(QString::fromStdString(session_data.name));
    emit updateId(session_data.id);

}


void CellEyeWindow::loadFrame() {

    if (session_data.data.find(current_frame) != session_data.data.end()) {
        current_frame_data = session_data.data[current_frame];
        if(current_frame_data.roi.area() ==0){
            current_frame_data.roi = cv::Rect(0, 0, images[0].cols, images[0].rows);
        }
        process_frame_state =current_frame_data.contour.empty();
    } else {
        process_frame_state = true;
    }

    emit updateIterations(current_frame_data.remove_noise_iterations);
    emit updateImgThreshold(current_frame_data.image_binary_threshold);
    emit activateBinaryThreshold(current_frame_data.manual_threshold);
    emit updateArea(current_frame_data.area);
    emit updateError(current_frame_data.error);
    emit updateState(process_frame_state);
    emit updateStopState(!process_frame_state);

}

void CellEyeWindow::saveData(bool update_session) {

    std::ofstream file(data_file_name);
    if (update_session) {
        session_data.data[current_frame] = current_frame_data;
    }
    file << serialize(session_data);
    file.close();

}

void CellEyeWindow::saveFile(bool checked) {


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
             << ",min_area"
             << ",area"
             << ",max_area"
             << ",error"
             << ",flag"
             << std::endl;


        for (auto &item : session_data.data) {

            file << session_data.name
                 << "," << session_data.id
                 << "," << item.first + 1
                 << "," << item.second.min_area
                 << "," << item.second.area
                 << "," << item.second.max_area
                 << "," << item.second.error
                 << "," << (item.second.activate_processing_flag ? "True" : "False")
                 << std::endl;
        }

        file.close();

    }

}

void CellEyeWindow::openFile(bool checked) {

    QString qt_fileName = QFileDialog::getOpenFileName(this, "Open Image", QDir::homePath(),
                                                       "Image Files (*.tiff *tif)");
    if (qt_fileName.isEmpty()) { return; }

    std::string fileName = qt_fileName.toStdString();

    LOGGER.info << "Opening file: " << fileName << std::endl;

    images.clear();


    if (cv::imreadmulti(fileName, images, cv::IMREAD_COLOR)) {

        boost::filesystem::path data_file = boost::filesystem::change_extension(fileName, ".json");
        data_file_name = data_file.string();
        LOGGER.debug << "data_file " << data_file << std::endl;
        if (boost::filesystem::exists(data_file)) {
            boost::property_tree::ptree data_tree;
            boost::property_tree::read_json(data_file_name, data_tree);
            session_data.deserialize(data_tree);
            process_frame_state = false;
        } else {
            session_data.name = boost::filesystem::basename(fileName);
            session_data.file_name = fileName;
            session_data.id = 1;
            session_data.data.clear();
            process_frame_state = true;
        }

        loadData();
        current_frame = 0;
        main_ui.progressBar->setMinimum(0);
        main_ui.progressBar->setMaximum(images.size());
        if (process_frame_state) {
            current_frame_data.roi = cv::Rect(0, 0, images[0].cols, images[0].rows);
        }
        loadFrame();
        update();

    } else {
        LOGGER.error << "Unable to open file: " << fileName << std::endl;
        QErrorMessage errorMessage(this);
        errorMessage.showMessage("Unable to open file:\n" + qt_fileName);
        errorMessage.exec();
    }

}

void CellEyeWindow::auto_process() {

    if (processing || images.empty()) {
        return;
    }
    processing = true;

    LOGGER.debug << "Auto process " << std::endl;

    QProgressDialog progress("Task in progress...", "Cancel", 0, images.size(), this);
    progress.setWindowModality(Qt::ApplicationModal);

    progress.setAutoClose(true);
    QVBoxLayout *layout = new QVBoxLayout;
    foreach (QObject *obj, progress.children()) {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget)
            layout->addWidget(widget);
    }
    progress.setLayout(layout);

    for (int i = current_frame + 1; i < images.size(); i++){
        progress.setValue(i);
        LOGGER.warning << "auto processing frame: " << i << std::endl;
        next();
        if(progress.wasCanceled()){break;}
    }

    processing=false;

}



void  CellEyeWindow::plot(){

    if(session_data.data.empty()){return;}
    saveData(true);
    QDialog dialog;
    plotDialog plot_ui(&dialog, session_data.data, current_frame, this);
    dialog.exec();

}

void  CellEyeWindow::preferences() {

    QDialog dialog;
    preferencesDialog preferences_ui(&dialog, processing_debug, preferences_file);
    dialog.exec();
    loadPreferences();

}


void CellEyeWindow::update() {

    if (images.empty()) { return; }

    main_ui.progressBar->setValue(current_frame + 1);

    LOGGER.debug << " frame " << current_frame << std::endl;


    //START PROCESS

    cv::Mat to_proc = images[current_frame](current_frame_data.roi);

    if (process_frame_state) {
        process(to_proc, current_frame_data);
        emit updateArea(current_frame_data.area);
        emit updateError(current_frame_data.error);
    }


    QGraphicsScene *scene = new QGraphicsScene();

    QImage q_image = Mat2QImage(images[current_frame]);
    ImageItem *item = new ImageItem(QPixmap::fromImage(q_image));
    item->setAcceptedMouseButtons(Qt::LeftButton);
    item->setAcceptHoverEvents(true);

    QObject::connect(item, SIGNAL(setPointer(double, double)), this, SLOT (setROIStart(double, double)));
    QObject::connect(item, SIGNAL(movePointer(double, double)), this, SLOT (moveROICorner(double, double)));
    QObject::connect(item, SIGNAL(unsetPointer(double, double)), this, SLOT (setROIEnd(double, double)));

    scene->addItem(item);

    QPen pen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    roiRectangle = scene->addRect(current_frame_data.roi.x, current_frame_data.roi.y,
                                                          current_frame_data.roi.width, current_frame_data.roi.height, pen);

    QPen path_pen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QColor errorColor(55, 55, 255, 125);
    QPen region_pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen ellipse_pen(Qt::blue, 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);

    auto contour = current_frame_data.contour;

    if(!contour.empty()) {
        QPainterPath contour_path(QPointF(current_frame_data.roi.x + contour[0].x, current_frame_data.roi.y + contour[0].y));
        for (int i = 1; i < contour.size(); i++) {
            contour_path.lineTo(current_frame_data.roi.x + contour[i].x, current_frame_data.roi.y + contour[i].y);
        }
        scene->addPath(contour_path, path_pen);
        auto fitted_ellipse = current_frame_data.ellipse;
        int width = fitted_ellipse.size.width;
        int height = fitted_ellipse.size.height;

        int max_ratio = current_frame_data.max_radio;
        int min_ratio = current_frame_data.min_radio;

        scene->addEllipse(current_frame_data.roi.x + fitted_ellipse.center.x - width / 2,
                                                          current_frame_data.roi.y + fitted_ellipse.center.y - height / 2, width,
                                                          height, ellipse_pen);

        QPainterPath ring_path;

        ring_path.addEllipse(current_frame_data.roi.x + fitted_ellipse.center.x - max_ratio ,
                          current_frame_data.roi.y + fitted_ellipse.center.y - max_ratio , 2*max_ratio,
                          2*max_ratio);

        ring_path.addEllipse(current_frame_data.roi.x + fitted_ellipse.center.x - min_ratio ,
                          current_frame_data.roi.y + fitted_ellipse.center.y - min_ratio , 2*min_ratio,
                          2*min_ratio);

        scene->addPath(ring_path, region_pen,QBrush(errorColor));
    }

    LOGGER.debug << "items:" << scene->items().count() << std::endl;

    if (main_ui.graphicsView->scene() != nullptr) {
        main_ui.graphicsView->scene()->clear();
    }
    main_ui.graphicsView->setScene(scene);

}


void CellEyeWindow::setFrame(int id){

    saveData(process_frame_state);
    if(images.empty()){return;}
    if (0 <= id && id < images.size()) { current_frame=id; }
    loadFrame();
    update();
    LOGGER.debug << "Set frame to: " << id << std::endl;
}


void CellEyeWindow::setROIStart(double x, double y) {

    LOGGER.debug << "setROIStart: " << x << "," << y << std::endl;
    if (roiRectangle != nullptr && process_frame_state) {

        new_roi.x = x;
        new_roi.y = y;
        new_roi.y = y;
        new_roi.width = 1;
        new_roi.height = 1;

        QPen pen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        main_ui.graphicsView->scene()->removeItem(roiRectangle);
        roiRectangle = main_ui.graphicsView->scene()->addRect(new_roi.x, new_roi.y, new_roi.width, new_roi.height, pen);
    }
}

void CellEyeWindow::moveROICorner(double x, double y) {

    if (roiRectangle != nullptr && process_frame_state) {
        main_ui.graphicsView->scene()->removeItem(roiRectangle);

        new_roi.width = abs(new_roi.x - x);
        new_roi.height = abs(new_roi.y - y);

        new_roi.x = std::min(static_cast<int>(std::round(x)), new_roi.x);
        new_roi.y = std::min(static_cast<int>(std::round(y)), new_roi.y);

        QPen pen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        roiRectangle = main_ui.graphicsView->scene()->addRect(new_roi.x, new_roi.y, new_roi.width, new_roi.height, pen);
    }

}

void CellEyeWindow::setROIEnd(double x, double y) {

    if (!process_frame_state) { return; }

    new_roi.width = abs(new_roi.x - x);
    new_roi.height = abs(new_roi.y - y);

    new_roi.x = std::min(static_cast<int>(std::round(x)), new_roi.x);
    new_roi.y = std::min(static_cast<int>(std::round(y)), new_roi.y);

    if (new_roi.width != 0 && new_roi.height != 0) { current_frame_data.roi = new_roi; }

    update();


}

void CellEyeWindow::process(const cv::Mat &image, frameData &data) {

    LOGGER.debug << "remove_noise_iterations: " << data.remove_noise_iterations << std::endl;
    LOGGER.debug << "image_binary_threshold: " << data.image_binary_threshold << std::endl;

    std::vector<std::vector<cv::Point> > contours;

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    if (processing_debug) {
        cv::imshow("gray", gray);
    }
    cv::Mat thresh;

    cv::Mat blur;
    cv::GaussianBlur(gray,blur,cv::Size2i(5,5),0);

    if(data.manual_threshold){
       cv::threshold(blur, thresh, data.image_binary_threshold, 255, cv::THRESH_BINARY_INV);
    }
    else {
        cv::threshold(blur, thresh, 0, 255,
                                     cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    }

    if (processing_debug) {
        cv::imshow("thresh", thresh);
    }

    cv::Mat opening;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(thresh, opening, cv::MORPH_OPEN, element, cv::Point(-1, -1), data.remove_noise_iterations);

    if (processing_debug) {
        cv::imshow("opening", opening);
    }

    cv::Mat sure_bg;
    cv::morphologyEx(opening, sure_bg, cv::MORPH_DILATE, element, cv::Point(-1, -1), data.remove_noise_iterations);

    if (processing_debug) {
        cv::imshow("sure_bg", sure_bg);
    }

    cv::Mat dist_transform;
    cv::distanceTransform(opening, dist_transform, cv::DIST_L2, 5);

    if (processing_debug) {
        cv::imshow("dist_transform", dist_transform);
    }

    cv::Mat sure_fg;
    //double min_val, max_val;
    //cv::minMaxIdx(dist_transform, &min_val, &max_val);
    //cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    dist_transform.convertTo(dist_transform, CV_8U);
    cv::threshold(dist_transform, sure_fg, 0, 255, cv::THRESH_BINARY+ cv::THRESH_TRIANGLE);
    sure_fg.convertTo(sure_fg, CV_8U);

    if (processing_debug) {
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


    cv::Mat image_8uc3;
    image.convertTo(image_8uc3, CV_8UC3);

    cv::watershed(image_8uc3, markers_32);


    cv::Mat mask(image_8uc3.rows, image_8uc3.cols, CV_8UC1);
    mask = 0;
    //mask.setTo(255, markers_32 == 2);
    mask.setTo(255, markers_32 >= 2);

    if (processing_debug) {
        cv::imshow("mask", mask);
    }


    findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    data.area = 0;
    data.contour.clear();
    for (auto &contour : contours) {
        LOGGER.debug << "contour size: " << contour.size() << std::endl;
        if(contour.size() < 5) {continue;}

        auto ellipse = cv::fitEllipse(contour);

        cv::Rect roi_shifted(0,0,data.roi.width,data.roi.height);
        cv::Point2f top_right(ellipse.size.width/2,ellipse.size.height/2);
        cv::Point2f bottom_right(ellipse.size.width/2,-ellipse.size.height/2);
        cv::Point2f top_left(-ellipse.size.width/2,ellipse.size.height/2);
        cv::Point2f bottom_left(-ellipse.size.width/2,-ellipse.size.height/2);

        if( !( roi_shifted.contains(ellipse.center)
        && roi_shifted.contains(ellipse.center+top_right)
        && roi_shifted.contains(ellipse.center+bottom_right)
        && roi_shifted.contains(ellipse.center+top_left)
        && roi_shifted.contains(ellipse.center+bottom_left)
        )){
            continue;
        }

        double area =  pi*0.25*ellipse.size.area();
        LOGGER.debug << " Ellipse area: " << area << std::endl;
        LOGGER.debug << " Contour area: " << cv::contourArea(contour) << std::endl;

        if(data.area <= area){
            data.area = area;
            data.contour = contour;
            data.ellipse = ellipse;
        }
    }

    if(data.area ==0){
        QErrorMessage errorMessage(this);
        errorMessage.showMessage("Image segmentation failed\n");
        errorMessage.setWindowModality(Qt::ApplicationModal);
        errorMessage.exec();
    }

    data.max_radio=0;
    data.min_radio=1e8;
    for(auto &point : data.contour){
        double dx = point.x - data.ellipse.center.x;
        double dy = point.y - data.ellipse.center.y;
        double distance = std::sqrt(dx*dx+dy*dy);
        data.max_radio = std::max(data.max_radio,distance);
        data.min_radio = std::min(data.min_radio,distance);
    }
    data.max_area = pi*data.max_radio*data.max_radio;
    data.min_area = data.min_radio < data.max_radio  ? pi*data.min_radio*data.min_radio : 0;
    data.error = data.max_area-data.min_area;

}





