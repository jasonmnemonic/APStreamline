#include "Camera.h"
#include <gst/gst.h>
#include <cstdlib>

class MJPGCamera : public Camera
{
protected:
    GstElement *encoder;
    GstElement *capsfilter;
    string encoder_name;
    guint32 bitrate;

public:
    MJPGCamera(string device, Quality q) : Camera(device, q)
    {
        encoder = nullptr;
        capsfilter = nullptr;
        Config camera_config;
        Config quality_config;

        // Read the file. If there is an error, report it and exit.
        try {
            cout << "Reading config" << endl;
            camera_config.readFile("config/MJPGCamera.cfg");
            quality_config.readFile("config/settings.cfg");
            read_configuration(camera_config.getRoot(), quality_config.getRoot());
        } catch (const FileIOException &fioex) {
            cerr << "I/O error while reading file." << std::endl;
        } catch (const ParseException &pex) {
            cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                 << " - " << pex.getError() << std::endl;
        }
    }
    virtual bool set_element_references(GstElement *pipeline) override
    {
        encoder = gst_bin_get_by_name(GST_BIN(pipeline), encoder_name.c_str());
        capsfilter = gst_bin_get_by_name(GST_BIN(pipeline), "capsfilter");
        if (encoder && capsfilter) {
            return true;
        }
        return false;
    }
    virtual bool set_bitrate(guint32 _bitrate) override
    {
        if (_bitrate < min_bitrate) {
            bitrate = min_bitrate;
        } else if (_bitrate > max_bitrate) {
            bitrate = max_bitrate;
        } else {
            bitrate = _bitrate;
        }
        g_object_set(G_OBJECT(encoder), "bitrate", bitrate, NULL);
        return true;
    }
    virtual bool set_quality(Quality q) override
    {
        string capsfilter_string;
        // TODO: add checks for if Q is valid or not
        current_quality = q;
        capsfilter_string = generate_capsfilter(current_quality);
        GstCaps *caps;
        caps = gst_caps_from_string(capsfilter_string.c_str());
        g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
        gst_caps_unref(caps);
        return true;
    }
    // call from ctr
    virtual bool read_configuration(Setting &camera_config, Setting &quality_config) override
    {
        Camera::read_configuration(camera_config, quality_config);
        // change this here
        encoder_name = static_cast<const char *>(camera_config.lookup("encoder_name"));
        return true;
    }
    virtual string generate_launch_string(Quality q) const override
    {
        string capsfilter_string;
        guint32 launch_bitrate;
        switch (q.get_quality_level()) {
        case Quality::QualityLevel::LOW:
            launch_bitrate = low_bitrate;
            break;
        case Quality::QualityLevel::MEDIUM:
            launch_bitrate = medium_bitrate;
            break;
        case Quality::QualityLevel::HIGH:
            launch_bitrate = high_bitrate;
            break;
        };
        capsfilter_string = generate_capsfilter(q);
        regex d("%device");
        regex cf("%capsfilter");
        regex enc("%encoder");
        regex br("%bitrate");
        regex_replace(launch_string, d, device_path);
        regex_replace(launch_string, cf, capsfilter_string);
        regex_replace(launch_string, enc, encoder_name);
        regex_replace(launch_string, br, to_string(launch_bitrate));
        return launch_string;
    }
    virtual void improve_quality(bool congested) override
    {
        set_bitrates_constants(congested);
        set_bitrate(bitrate + increment_bitrate);
        if (current_quality.get_quality_level() == Quality::QualityLevel::LOW && bitrate > medium_bitrate) {
            set_quality(Quality::get_quality(Quality::QualityLevel::MEDIUM));
        } else if (current_quality.get_quality_level() == Quality::QualityLevel::MEDIUM && bitrate > high_bitrate) {
            set_quality(Quality::get_quality(Quality::QualityLevel::HIGH));
        }
    }
    virtual void degrade_quality(bool congested) override
    {
        set_bitrates_constants(congested);
        set_bitrate(bitrate - decrement_bitrate);
        if (current_quality.get_quality_level() == Quality::QualityLevel::MEDIUM && bitrate < medium_bitrate) {
            set_quality(Quality::get_quality(Quality::QualityLevel::LOW));
        } else if (current_quality.get_quality_level() == Quality::QualityLevel::HIGH && bitrate < high_bitrate) {
            set_quality(Quality::get_quality(Quality::QualityLevel::MEDIUM));
        }
    }
};