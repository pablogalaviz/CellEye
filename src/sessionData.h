//
// Created by pablo on 7/9/20.
//

#ifndef CELLEYE_SESSIONDATA_H
#define CELLEYE_SESSIONDATA_H

#include <boost/lexical_cast.hpp>
#include <string>
#include <sstream>
#include "logger.h"
#include "utils.h"
#include "frameData.h"

struct sessionData {

    std::string file_name;
    std::string name;
    int id;
    std::map<int,frameData> data;


    virtual std::string serialize() const {
        std::stringstream result;
        result << "{";
        result << "\"id\":" << id;
        result << ",\"name\":\"" << name << "\"";
        result << ",\"file_name\":\"" << file_name << "\"";
        result << ",\"data\":" << serialize_map(data);
        result << "}";
        return result.str();
    };

    virtual void deserialize(const boost::property_tree::ptree &tree) {
        try {
            id = tree.get<int>("id");
            name = tree.get<std::string>("name");
            file_name = tree.get<std::string>("file_name");

            for(auto &item: tree.get_child("data")){
                frameData new_frame;
                new_frame.deserialize(item.second);
                data[std::stoi(item.first)] = new_frame;
            }


        }
        catch (std::exception &e) {
            LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
            LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
            exit(EIO);
        }
    }



};


inline std::ostream& operator<<(std::ostream& result, const sessionData& rhs)
{
    result << rhs.serialize();
    return result;
}





#endif //CELLEYE_SESSIONDATA_H
