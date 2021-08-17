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

    int remove_noise_iterations = 10;
    int image_binary_threshold = 125;
    int manual_threshold=false;
    double area = 0;
    double max_area = 0;
    double min_area = 0;
    double error = 0;
    bool activate_processing_flag=false;
    std::vector<cv::Point>  contour;
    cv::Rect roi;
    cv::RotatedRect ellipse;
    double max_radio;
    double min_radio;

    virtual std::string serialize() const {
        std::stringstream result;
        result << "{";
        result << "\"remove_noise_iterations\":" << remove_noise_iterations;
        result << ",\"image_binary_threshold\":" << image_binary_threshold;
        result << ",\"manual_threshold\":" << manual_threshold;
        result << ",\"area\":" << area;
        result << ",\"max_area\":" << max_area;
        result << ",\"min_area\":" << min_area;
        result << ",\"error\":" << error;
        result << ",\"max_radio\":" << max_radio;
        result << ",\"min_radio\":" << min_radio;

        result << ",\"roi\":{";
        result << "\"x\":" << roi.x;
        result << ",\"y\":" << roi.y;
        result << ",\"width\":" << roi.width;
        result << ",\"height\":" << roi.height;
        result << "}";

        result << ",\"ellipse\":{";
        result << "\"x\":" << ellipse.center.x;
        result << ",\"y\":" << ellipse.center.y;
        result << ",\"width\":" << ellipse.size.width;
        result << ",\"height\":" << ellipse.size.height;
        result << ",\"angle\":" << ellipse.angle;
        result << "}";

        result << ",\"contour\":[";
        for (int i = 0; i < contour.size(); i++) {
            result << (i == 0 ? "{" : ",{");
            result << "\"x\":" << contour[i].x;
            result << ",\"y\":" << contour[i].y;
            result << "}";
        }
        result << "]";
        result << "}";
        return result.str();
    };

    virtual void deserialize(const boost::property_tree::ptree &tree) {
        try {
            remove_noise_iterations = tree.get<int>("remove_noise_iterations");
            image_binary_threshold = tree.get<int>("image_binary_threshold");
            manual_threshold = tree.get<int>("manual_threshold");
            area = tree.get<double>("area");
            max_area = tree.get<double>("max_area");
            min_area = tree.get<double>("min_area");
            error = tree.get<double>("error");
            max_radio = tree.get<double>("max_radio");
            min_radio = tree.get<double>("min_radio");

            roi.x = tree.get<int>("roi.x");
            roi.y = tree.get<int>("roi.y");
            roi.width = tree.get<int>("roi.width");
            roi.height = tree.get<int>("roi.height");

            ellipse.center.x=tree.get<double>("ellipse.x");
            ellipse.center.y=tree.get<double>("ellipse.y");
            ellipse.size.width = tree.get<double>("ellipse.width");
            ellipse.size.height = tree.get<double>("ellipse.height");
            ellipse.angle = tree.get<double>("ellipse.angle");

            deserialize_d(tree.get_child("contour"),contour);
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
