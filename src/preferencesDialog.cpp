//
// Created by pablo on 25/6/21.
//

#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "preferencesDialog.h"


preferencesDialog::preferencesDialog( QDialog *parent , bool _debug, std::string _file_name){

    debug =_debug;
    file_name = _file_name;
    preferences_ui.setupUi(parent);

    //        QObject::connect(buttonBox, SIGNAL(accepted()), DialogPreference, SLOT(accept()));
    QObject::connect(preferences_ui.maximum_method_iterations_spinBox, SIGNAL (valueChanged(int)), this, SLOT (maximum_method_iterations_changed(int)));
    QObject::connect(preferences_ui.maximum_automated_iterations_spinBox, SIGNAL (valueChanged(int)), this, SLOT (maximum_automated_iterations_changed(int)));
    QObject::connect(preferences_ui.area_tolerance_doubleSpinBox, SIGNAL (valueChanged(double)), this, SLOT (area_tolerance_changed(double)));
    QObject::connect(preferences_ui.buttonBox, SIGNAL (accepted()), this, SLOT (save()));

    if (boost::filesystem::exists(file_name)) {
        LOGGER.debug << "reading preference file: " << file_name <<std::endl;
        boost::property_tree::ptree data_tree;
        boost::property_tree::read_json(file_name, data_tree);
        preference_data.deserialize(data_tree);
    }
    else{
        preference_data.maximum_method_iterations = preferences_ui.maximum_method_iterations_spinBox->value();
        preference_data.maximum_automated_iterations = preferences_ui.maximum_automated_iterations_spinBox->value();
        preference_data.area_tolerance = preferences_ui.area_tolerance_doubleSpinBox->value();
    }

    preferences_ui.maximum_method_iterations_spinBox->setValue(preference_data.maximum_method_iterations);
    preferences_ui.maximum_automated_iterations_spinBox->setValue(preference_data.maximum_automated_iterations);
    preferences_ui.area_tolerance_doubleSpinBox->setValue(preference_data.area_tolerance);

}

