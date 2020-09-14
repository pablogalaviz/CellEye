//
// Created by pablo on 12/7/20.
//

#ifndef CELLEYE_UTILS_H
#define CELLEYE_UTILS_H

#include <chrono>
#include "logger.h"
#include <boost/property_tree/ptree.hpp>


inline void create_output_directory(std::string path, bool backup) {

    int sys_out;
    if (backup) {
        std::string rmdir = "rm -rf " + path + "_prev";
        sys_out = system(rmdir.c_str());
        if (sys_out) {
            LOGGER.debug << "rm -rf return value " << sys_out << std::endl;
        }
        std::string mvdir = "mv -f " + path + " " + path + "_prev 2>/dev/null";
        sys_out = system(mvdir.c_str());
        if (sys_out) {
            LOGGER.debug << "mv -f return value " << sys_out << std::endl;
        }
    } else {
        std::string rmdir = "rm -rf " + path;
        sys_out = system(rmdir.c_str());
    }

    std::string mkdir = "mkdir " + path;
    sys_out = system(mkdir.c_str());

}

inline void log_command(std::string path, const int ac, char *av[]) {

    std::string cmd = "echo '#!/bin/bash' > " + path + "/command.sh";
    int sys_out = system(cmd.c_str());

    cmd = "echo cd `pwd` >> " + path + "/command.sh";
    sys_out = system(cmd.c_str());

    std::stringstream param;

    for (int i = 0; i < ac; i++)

        param << av[i] << " ";

    cmd = "echo " + param.str() + " >> " + path + "/command.sh;";
    sys_out = system(cmd.c_str());


    cmd = "chmod +x " + path + "/command.sh";
    sys_out = system(cmd.c_str());


}

inline std::string get_time_str(long value, const std::string& unit)
{

    std::stringstream result;

    if (value > 0)
    {
        result << value << " " << unit;
        if (value > 1)
            result << "s ";
        else
            result << " ";
    }
    return result.str();
}

inline void goodbye( std::chrono::system_clock::time_point start){
    auto stop = std::chrono::system_clock::now();

    auto delta_hours = std::chrono::duration_cast<std::chrono::hours>(stop - start).count();
    auto delta_minutes = std::chrono::duration_cast<std::chrono::minutes>(stop - start).count();
    auto delta_seconds = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();
    auto delta_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    int days = int(delta_hours/24);
    auto hours = delta_hours-days*24;
    auto minutes = delta_minutes - delta_hours*60;
    auto seconds = delta_seconds - delta_minutes*60;
    auto milliseconds = delta_milliseconds - delta_seconds*1000;

    LOGGER.info << "Finished in "
                << get_time_str(days, "day")
                << get_time_str(hours, "hour")
                << get_time_str(minutes, "minute")
                << get_time_str(seconds, "second")
                << milliseconds << " milliseconds " << std::endl;

    LOGGER.info << "All done! " << std::endl;

}


template<class T>
inline std::string serialize(const T &item) {
    std::stringstream sample_data;
    sample_data << item;
    return sample_data.str();
};

/*
template<class T>
void deserialize(const boost::property_tree::ptree &tree,T &item) {
    item.deserialize(tree);
}

template<>
inline void deserialize(const boost::property_tree::ptree &tree,unsigned int &value) {
    value = tree.get_value<unsigned int>();
}
*/

template<class T>
std::string serialize(const std::vector<T> &vector_data) {
    std::stringstream result;
    result << "[";
    int list_size= vector_data.size();
    for(int i=0; i < list_size; i++){
        result << serialize(vector_data[i]) << (i<list_size-1 ? "," :"");
    }
    result << "]";
    return result.str();
};



template<class S, class T>
std::string serialize_map(const std::map<S,T> &map_data) {
    std::stringstream result;
    result << "{";
    for(auto iter_data = map_data.begin(); iter_data != map_data.end(); ++iter_data){
        result << "\"" << iter_data->first << "\":";
        result << serialize(iter_data->second);
        result << (std::next(iter_data) != map_data.end() ? "," : "");
    }
    result << "}";

    return result.str();
};


template<class T>
void deserialize( const boost::property_tree::ptree &tree,std::map<std::string, T> &result) {

    try {
        for(auto &t : tree) {
            T new_item;
            deserialize(t.second,new_item);
            result[t.first]=new_item;
        }
    }
    catch (std::exception &e) {
        LOGGER.error << "From: " << __FILE__ << ":" << __LINE__ << std::endl;
        LOGGER.error << "Exception parsing json file: " << e.what() << std::endl;
        exit(EIO);
    }


}




#endif //CELLEYE_UTILS_H
