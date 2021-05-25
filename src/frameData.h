//
// Created by pablo on 7/9/20.
//

#ifndef CELLEYE_FRAMEDATA_H
#define CELLEYE_FRAMEDATA_H

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <sstream>
#include <opencv2/core/types.hpp>
#include "logger.h"
#include "utils.h"

inline void deserialize_d(const boost::property_tree::ptree &tree, cv::Point &value) {
    try {
        value.x = tree.get<int>("x");
        value.y = tree.get<int>("y");
    }
    catch (std::exception &e) {
        LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
        LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
        exit(EIO);
    }
}


template<class T>
void deserialize_d( const boost::property_tree::ptree &tree, std::vector<T> & result) {
    try {
        for(auto &t : tree) {
            T new_item;
            deserialize_d(t.second,new_item);
            result.push_back(new_item);
        }
    }
    catch (std::exception &e) {
        LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
        LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
        exit(EIO);
    }

}


struct frameData {

    int iterations = 10;
    double threshold = 0.7;
    int img_threshold = 125;
    int error = 5;
    double area_pixel = 0;
    double error_area_pixel = 0;
    double area_path = 0;
    double error_area_path = 0;
    bool flag=false;
    std::vector<std::vector<cv::Point> > contours;
    cv::Rect roi;

    virtual std::string serialize() const {
        std::stringstream result;
        result << "{";
        result << "\"iterations\":" << iterations;
        result << ",\"threshold\":" << threshold;
        result << ",\"img_threshold\":" << img_threshold;
        result << ",\"error\":" << error;
        result << ",\"area_pixel\":" << area_pixel;
        result << ",\"error_area_pixel\":" << error_area_pixel;
        result << ",\"area_path\":" << area_path;
        result << ",\"error_area_path\":" << error_area_path;
        result << ",\"roi\":{";
        result << "\"x\":" << roi.x;
        result << ",\"y\":" << roi.y;
        result << ",\"width\":" << roi.width;
        result << ",\"height\":" << roi.height;
        result << "}";

        result << ",\"contours\":[";

        bool first = true;
        for (auto &item : contours) {
            result << (first ? "[" : ",[");
            for (int i = 0; i < item.size(); i++) {
                result << (i == 0 ? "{" : ",{");
                result << "\"x\":" << item[i].x;
                result << ",\"y\":" << item[i].y;
                result << "}";
            }
            first = false;
            result << "]";
        }
        result << "]";
        result << "}";
        return result.str();
    };

    virtual void deserialize(const boost::property_tree::ptree &tree) {
        try {
            iterations = tree.get<int>("iterations");
            threshold = tree.get<double>("threshold");
            img_threshold = tree.get<double>("img_threshold");
            error = tree.get<int>("error");
            area_path = tree.get<double>("area_path");
            error_area_path = tree.get<double>("error_area_path");
            area_pixel = tree.get<double>("area_pixel");
            error_area_pixel = tree.get<double>("error_area_pixel");
            roi.x = tree.get<int>("roi.x");
            roi.y = tree.get<int>("roi.y");
            roi.width = tree.get<int>("roi.width");
            roi.height = tree.get<int>("roi.height");
            deserialize_d(tree.get_child("contours"),contours);
        }
        catch (std::exception &e) {
            LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
            LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
            exit(EIO);
        }
    }


};


inline std::ostream &operator<<(std::ostream &result, const frameData &rhs) {
    result << rhs.serialize();
    return result;
}


#endif //CELLEYE_FRAMEDATA_H
