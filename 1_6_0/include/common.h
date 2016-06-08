#ifndef __CYCLOPS_COMMON_H__
#define __CYCLOPS_COMMON_H__

/*
 *  These are meant to match up with the enum from imxv4l2videosrc with an ov5640 sensor.
 */
typedef enum {
    VideoMode_VGA = 0,
    VideoMode_QVGA = 1,
    VideoMode_NTSC = 2,
    VideoMode_PAL = 3,
    VideoMode_720P = 4,
    VideoMode_1080P = 5,
} VideoMode;

#endif /* __CYCLOPS_COMMON_H__ */