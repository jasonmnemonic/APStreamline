#ifndef CAMERA_H
#define CAMERA_H

#include <utility>
#include <string>
#include <unordered_map>
#include <gst/gst.h>
#include "Quality.h"
#include <libconfig.h++>
#include <iostream>
#include <regex>

using namespace std;
using namespace libconfig;

class Camera
{
protected:
    string device_path;
    string launch_string;
    string camera_name;
    string capsfilter;
    uint32_t frame_property_bitmask;

    bool fallback;
    bool dynamic_res;
    bool dynamic_bitrate;
    unordered_map<string, bool> encoder_params_bool;
    unordered_map<string, int> encoder_params_int;

    int default_framerate;
    int default_res;

    guint32 steady_state_min;
    guint32 steady_state_max;
    guint32 steady_state_increment;
    guint32 steady_state_decrement;

    guint32 congested_state_min;
    guint32 congested_state_max;
    guint32 congested_state_increment;
    guint32 congested_state_decrement;

    guint32 low_bitrate;
    guint32 medium_bitrate;
    guint32 high_bitrate;

    guint32 min_bitrate;
    guint32 max_bitrate;
    guint32 increment_bitrate;
    guint32 decrement_bitrate;

    Quality current_quality;

public:
    Camera(string device, Quality q) : device_path(device), current_quality(q)
    {
    }
    virtual ~Camera() {}
    virtual bool set_element_references(GstElement *pipeline) = 0;
    virtual bool set_bitrate(guint32 _bitrate)
    {
        return dynamic_bitrate;
    }
    virtual bool set_quality(Quality q)
    {
        return dynamic_res;
    }
    Quality get_quality()
    {
        return current_quality;
    }
    string get_device_path()
    {
        return device_path;
    }
    void set_bitrates_constants(bool congested)
    {
        if (congested) {
            min_bitrate = congested_state_min;
            max_bitrate = congested_state_max;
            increment_bitrate = congested_state_increment;
            decrement_bitrate = congested_state_decrement;
        } else {
            min_bitrate = steady_state_min;
            max_bitrate = steady_state_max;
            increment_bitrate = steady_state_increment;
            decrement_bitrate = steady_state_decrement;
        }
    }
    virtual bool read_configuration(Setting& camera_config, Setting& quality_config)
    {
        // Setting &quality_config = config_root["quality"];
        // string camera_name_path = "cameras." + camera_name;
        // Setting &camera_config = config_root[camera_name_path.c_str()];

        steady_state_min = quality_config.lookup("quality.steady_state.min");
        steady_state_max = quality_config.lookup("quality.steady_state.max");
        steady_state_increment = quality_config.lookup("quality.steady_state.increment");
        steady_state_decrement = quality_config.lookup("quality.steady_state.decrement");

        congested_state_min = quality_config.lookup("quality.congested_state.min");
        congested_state_max = quality_config.lookup("quality.congested_state.max");
        congested_state_increment = quality_config.lookup("quality.congested_state.increment");
        congested_state_decrement = quality_config.lookup("quality.congested_state.decrement");

        low_bitrate = quality_config.lookup("quality.bitrate.low");
        medium_bitrate = quality_config.lookup("quality.bitrate.med");
        high_bitrate = quality_config.lookup("quality.bitrate.high");

        launch_string = static_cast<const char *>(camera_config.lookup("camera.properties.launch_string"));
        capsfilter = static_cast<const char *>(camera_config.lookup("camera.properties.capsfilter"));
        fallback = camera_config.lookup("camera.properties.fallback");
        dynamic_res = camera_config.lookup("camera.properties.dynamic_res");
        dynamic_bitrate = camera_config.lookup("camera.properties.dynamic_bitrate");
        default_framerate = camera_config.lookup("camera.properties.default_framerate");

        Setting& encoder_params = camera_config.lookup("camera.encoder_params");
        int val = encoder_params.getLength();


        for (int i = 0; i < encoder_params.getLength(); i++) {
            string key;
            Setting::Type type;
            key = encoder_params[i].getName();
            type = encoder_params[i].getType();
            switch (type) {
            case Setting::Type::TypeBoolean:
                encoder_params_bool[key] = encoder_params.lookup(key);
                break;
            case Setting::Type::TypeInt:
                encoder_params_int[key] = encoder_params.lookup(key);
                break;
            default:
                cerr << "Other types not implemented yet" << endl;
                return false;
            }
        }
        return true;
    }
    virtual string generate_capsfilter(Quality q) const
    {
        regex w("%width");
        regex h("%height");
        regex f("%framerate");
        string caps;
        caps = capsfilter;
        caps = regex_replace(caps, w, to_string(q.getWidth()));
        caps = regex_replace(caps, h, to_string(q.getHeight()));
        caps = regex_replace(caps, f, to_string(q.getFramerate()));
        return caps;
    }
    virtual string generate_launch_string(Quality q) const = 0;
    virtual void improve_quality(bool congested) = 0;
    virtual void degrade_quality(bool congested) = 0;
    bool dynamic_res_capability() const
    {
        return dynamic_res;
    }
    bool dynamic_bitrate_capability() const
    {
        return dynamic_bitrate;
    }
};

#endif