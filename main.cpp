#include <atomic>
#include <boost/program_options.hpp>
#include <csignal>
#include <iostream>
#include "src/utils.h"
#include "src/window.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QDebug>
#include <QtWidgets/QPushButton>
//#include <QtUiTools/QtUiTools>


int main(int ac, char *av[]) {
    try {

        auto start_time = std::chrono::system_clock::now();

        bool backup;
        std::string output_path;
        boost::program_options::options_description genericOptions(
                "FishEye.  \nAllowed common:");
        genericOptions.add_options()
                ("backup", boost::program_options::value<bool>(&backup)->default_value(true), "Create a backup of previous output")
                ("debug,d", "Shows debug messages in log")
                ("debug-image,i", "Shows image processing steps")
                ("output_path,o", boost::program_options::value<std::string>(&output_path)->default_value("output"), "Output directory name")
                ("help,h", "Shows a help message")
                ("silent,s", "Shows only errors");

        boost::program_options::options_description cmdlineOptions;
        cmdlineOptions.add(genericOptions);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(ac, av).options(cmdlineOptions).run(),vm);
        boost::program_options::notify(vm);

        if (vm.count("help")) {
            std::cerr << cmdlineOptions << std::endl;
            return 0;
        }


        create_output_directory(output_path, backup);
        log_command(output_path, ac, av);
        std::string log_file = output_path + "/output.log";

        bool debug = vm.count("debug");
        bool silent = vm.count("silent");
        if (silent)
            LOGGER.init(log_t::ERROR, log_file);
        else {
            if (debug)
                LOGGER.init(log_t::DBG, log_file);
            else
                LOGGER.init(log_t::INFO, log_file);
        }

        QApplication app (ac, av);

        QMainWindow widget;

        bool debug_image = vm.count("debug-image");

        Window mainWindow(&widget, debug_image);

        widget.show();

        auto result =  app.exec();

        goodbye(start_time);

        return result;
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
        return -1;
    }

    return 0;


}
