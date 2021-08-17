//
// Created by pablo on 25/6/21.
//

#ifndef CELLEYE_PREFERENCESDIALOG_H
#define CELLEYE_PREFERENCESDIALOG_H

#include <qt/ui_preferences.h>
#include "utils.h"

struct preferenceData {

    int maximum_method_iterations=10;
    int maximum_automated_iterations=1000;
    double area_tolerance=0.5;

    virtual void deserialize(const boost::property_tree::ptree &tree) {
        try {
            maximum_method_iterations = tree.get<int>("maximum_method_iterations");
            area_tolerance = tree.get<double>("area_tolerance");
            maximum_automated_iterations = tree.get<int>("maximum_automated_iterations");
        }
        catch (std::exception &e) {
            LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
            LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
            exit(EIO);
        }
    }

    virtual std::string serialize() const {
        std::stringstream result;
        result << "{";
        result << "\"maximum_method_iterations\":" << maximum_method_iterations;
        result << ",\"area_tolerance\":" << area_tolerance;
        result << ",\"maximum_automated_iterations\":" << maximum_automated_iterations;
        result << "}";
        return result.str();
    }

    };

inline std::ostream &operator<<(std::ostream &result, const preferenceData &rhs) {
    result << rhs.serialize();
    return result;
}

class preferencesDialog : public QObject{
    Q_OBJECT

    std::string file_name;
    bool debug;
    Ui::DialogPreference preferences_ui;
    preferenceData preference_data;

public:
    explicit preferencesDialog(QDialog *parent = 0, bool _debug = false, std::string _file_name="");

protected slots:

    void maximum_method_iterations_changed(int value) {
        LOGGER.debug << "maximum method remove_noise_iterations changed: " << value << std::endl;
        preference_data.maximum_method_iterations=value;
    }
    void maximum_automated_iterations_changed(int value) {
        LOGGER.debug << "maximum automated remove_noise_iterations changed: " << value << std::endl;
        preference_data.maximum_automated_iterations = value;
}
    void area_tolerance_changed(double value) {
        LOGGER.debug << "area tolerance changed: " << value << std::endl;
        preference_data.area_tolerance=value;
    }

    void save(){
        LOGGER.debug << "save preferences "<< file_name <<std::endl;
        std::ofstream file(file_name);
        file << serialize(preference_data);
        file.close();
    }

};



#endif //CELLEYE_PREFERENCESDIALOG_H
