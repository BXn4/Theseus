#ifndef _AV_H
#define _AV_H

// AV pack and video standard bits the dashboard reads out of the console's
// stored AV setting to work out the connected cable and available modes.

// AV pack (low byte)
#define AV_PACK_NONE          0x00000000
#define AV_PACK_STANDARD      0x00000001
#define AV_PACK_RFU           0x00000002
#define AV_PACK_SCART         0x00000003
#define AV_PACK_HDTV          0x00000004
#define AV_PACK_VGA           0x00000005
#define AV_PACK_SVIDEO        0x00000006
#define AV_PACK_MASK          0x000000FF

// Region / standard (second byte)
#define AV_STANDARD_NTSC_M    0x00000100
#define AV_STANDARD_NTSC_J    0x00000200
#define AV_STANDARD_PAL_I     0x00000300
#define AV_STANDARD_PAL_M     0x00000400
#define AV_STANDARD_MASK      0x0000FF00

// Aspect / HDTV / refresh flags
#define AV_FLAGS_WIDESCREEN   0x00010000
#define AV_FLAGS_LETTERBOX    0x00100000
#define AV_FLAGS_HDTV_480i    0x00000000
#define AV_FLAGS_HDTV_720p    0x00020000
#define AV_FLAGS_HDTV_1080i   0x00040000
#define AV_FLAGS_HDTV_480p    0x00080000
#define AV_FLAGS_INTERLACED   0x00200000
#define AV_FLAGS_60Hz         0x00400000
#define AV_FLAGS_50Hz         0x00800000

#endif // _AV_H
