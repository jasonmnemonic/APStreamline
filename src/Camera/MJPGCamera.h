#ifndef MJPG_CAMERA_H
#define MJPG_CAMERA_H

#include "Camera.h"
#include <gst/gst.h>

class MJPGCamera : public Camera
{
protected:
    GstElement *encoder;
    GstElement *capsfilter;
    bool get_supported_qualities();

public:
    MJPGCamera(string device, Quality q);
    virtual bool set_element_references(GstElement *pipeline) override;
    virtual bool set_bitrate(guint32 _bitrate) override;
    virtual bool set_quality(Quality q) override;
    virtual string generate_launch_string() const override;
};

#endif