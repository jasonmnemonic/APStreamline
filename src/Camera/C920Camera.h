#ifndef C920_CAMERA_H
#define C920_CAMERA_H

#include "Camera.h"
#include <gst/gst.h>

class C920Camera : public Camera
{
protected:
    GstElement *device;
    GstElement *capsfilter;
    virtual bool read_configuration(Setting &camera_config, Setting &quality_config) override;

public:
    C920Camera(string device, Quality q);
    virtual bool set_element_references(GstElement *pipeline) override;
    virtual bool set_bitrate(guint32 _bitrate) override;
    virtual bool set_quality(Quality q) override;
    virtual string generate_launch_string() const override;
};

#endif