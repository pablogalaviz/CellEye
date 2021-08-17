//
// Created by pablo on 12/7/20.
//

#ifndef CELLEYE_CELLEYE_H
#define CELLEYE_CELLEYE_H

#include <QWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QMainWindow>
#include <qt/ui_main.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "sessionData.h"
#include <boost/system/system_error.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/filesystem.hpp>
#include <QProgressDialog>
#include <QGraphicsEllipseItem>

const double pi = boost::math::constants::pi<double>();

class CellEyeWindow : public QWidget {
Q_OBJECT

    Ui::MainWindow main_ui;
    std::vector<cv::Mat> images;
    int current_frame;
    sessionData session_data;
    std::string data_file_name;
    bool process_frame_state;
    bool processing = false;

    frameData current_frame_data;
    std::string preferences_file;
    bool processing_debug;

public:
    /// Creates the main UI application.
    /// \param parent [in] main window widget
    /// \param _debug [in] activate processing debugging.
    /// \param _preferences_file [in] define the preference file path
    explicit CellEyeWindow(QMainWindow *parent = 0, bool _debug = false, std::string _preferences_file = "");

protected slots:

    /// Opens a QT File Dialog to open a tiff image.
    /// \param checked
    void openFile(bool checked);

    /// Opens a QT Save File dialog to save the data into a csv file.
    /// \param checked
    void saveFile(bool checked);

    /// Changes the name of the data.
    /// \param value [In] name of the data
    void changeName(QString value) {
        session_data.name = value.toStdString();
    }

    /// Changes the ID of the data.
    /// \param value [In] new id
    void changeId(int value) {
        session_data.id = value;
    }

    /// Toggles processing mode
    /// \param value [In] true for processing, false for locked.
    void changeProcessState(bool value) {
        process_frame_state = value;
        if (!value) {
            saveData(true);
        }
        LOGGER.debug << "process state: " << (value ? "true" : "false") << std::endl;
    }

    /// Flags a particular data point
    /// \param value [In] true if the data should be flagged.
    void changeFlagState(bool value) {
        current_frame_data.activate_processing_flag = value;
        LOGGER.debug << "flag state: " << (value ? "true" : "false") << std::endl;
    }

    /// Changes the number of iteration in the noise removal function.
    /// \param value [In] new value (>=1)
    void changeRemoveNoiseIterations(int value) {
        current_frame_data.remove_noise_iterations = std::max(value,1);
        update();
    }

    /// Sets the minimum value of the image thresholding (0,255).
    /// \param value [In] New threshold alue.
    void changeImageBinaryThreshold(int value) {
        current_frame_data.image_binary_threshold = std::max(std::min(value,255),0);
        update();
    }

    void activateBinaryThreshold(int value){
        current_frame_data.manual_threshold = value;
                //== Qt::CheckState::Checked;
        update();
    }

    /// Sets the region of interest's starting point.
    /// \param x [In] abscissa value
    /// \param y [In] ordinate value
    void setROIStart(double x, double y);

    /// Moves the region of interest's corner.
    /// \param x [In] abscissa value
    /// \param y [In] ordinate value
    void moveROICorner(double x, double y);

    /// Sets the region of interest's ending point.
    /// \param x [In] abscissa value
    /// \param y [In] ordinate value
    void setROIEnd(double x, double y);

    /// Sets the active frame
    /// \param id [In] id of the frame
    void setFrame(int id);

    /// Updates the current frame,
    void update();

    /// Switch to the next frame
    void next() {
        saveData(process_frame_state);
        if (images.empty()) { return; }
        if (current_frame < images.size() - 1) { current_frame++; }
        loadFrame();
        update();
    }


    /// Switch to the previous frame
    void previous() {
        saveData(process_frame_state);
        if (images.empty()) { return; }
        if (current_frame > 0) { current_frame--; }
        loadFrame();
        update();
    }


    /// Starts auto processing mode
    void auto_process();

    /// Opens the plot window
    void plot();

    /// Opens the preferences window
    void preferences();

signals:

    /// Triggers an update signal to update the AreaLcdNumber display widget.
    /// \param value [in] New value
    void updateArea(double value);

    /// Triggers an update signal to update the ErrorLcdNumber display widget.
    /// \param value [in] New value
    void updateError(double value);

    /// Triggers an update signal to update the experimentNameLineEdit widget.
    /// \param value [in] New value
    void updateName(QString value);

    /// Triggers an update signal to update the experimentIdSpinBox widget.
    /// \param value [in] New value
    void updateId(int value);

    /// Triggers an update signal to update the processPushButton widget.
    /// \param value [in] New value
    void updateState(bool value);

    /// Triggers an update signal to update the processStopPushButton widget.
    /// \param value [in] New value
    void updateStopState(bool value);

    /// Triggers an update signal to update the RemoveNoiseIterationsSpinBox widget.
    /// \param value [in] New value
    void updateIterations(int value);

    void updateThreshold(int value);

    void updateImgThreshold(int value);

    void updateErrorRegion(int value);

private:

    cv::Rect new_roi;
    QGraphicsRectItem *roiRectangle;

    void process(const cv::Mat &image, frameData &data);

    void loadData();

    void saveData(bool update_session);

    void loadFrame();

    void loadPreferences();


};


#endif //CELLEYE_CELLEYE_H
