#ifndef QOS_ESTIMATOR_H
#define QOS_ESTIMATOR_H

#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <gst/rtp/gstrtcpbuffer.h>
#include <sys/time.h>

#include "QoSReport.h"
#include "NTPTime.h"

class QoSEstimator {
private:
    gfloat smooth_rtt;
    guint64 prev_rr_time;
    guint32 prev_pkt_count;
    gfloat prev_buffer_occ;
    gfloat rtp_size;
    guint32 bytes_transferred;
    // not the same as encoding bitrate!
    const guint32* h264_bitrate; // maybe there's a better way than ptr
    timeval prev_tv;

    gfloat estimated_bitrate;
    gfloat encoding_bitrate;
    gfloat smooth_enc_bitrate;
    QoSReport qos_report;


    guint64 get_current_ntp_time();
    guint32 get_compressed_ntp_time(const guint64 &full_ntp_timestamp);
    void process_rr_packet(GstRTCPPacket* packet);
    void process_sr_packet(GstRTCPPacket* packet);
    static void exp_smooth_val(const gfloat &curr_val, gfloat &smooth_val, gfloat alpha);
public:
    QoSEstimator();
    QoSEstimator(guint32* bitrate);
    ~QoSEstimator();
    QoSReport get_qos_report();
    void estimate_rtp_pkt_size(const guint32 &pkt_size);
    void estimate_encoding_rate(const guint32 &pkt_size);
    void handle_rtcp_packet(GstRTCPPacket* packet);
};

#endif