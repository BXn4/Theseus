#ifndef _XLAUNCH_H_
#define _XLAUNCH_H_

// Layout of the launch data page passed between titles at boot. The dashboard
// reads the previous title's launch type, title id, and payload out of it.

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#define MAX_LAUNCH_DATA_SIZE 3072

#define LDT_LAUNCH_DASHBOARD 1
#define LDT_NONE             0xFFFFFFFF

typedef struct _LAUNCH_DATA_HEADER
{
    ULONG dwLaunchDataType;
    ULONG dwTitleId;
    CHAR  szLaunchPath[520];
    ULONG dwFlags;
} LAUNCH_DATA_HEADER, *PLAUNCH_DATA_HEADER;

typedef struct _LAUNCH_DATA_PAGE
{
    LAUNCH_DATA_HEADER Header;
    UCHAR Pad[PAGE_SIZE - MAX_LAUNCH_DATA_SIZE - sizeof(LAUNCH_DATA_HEADER)];
    UCHAR LaunchData[MAX_LAUNCH_DATA_SIZE];
} LAUNCH_DATA_PAGE, *PLAUNCH_DATA_PAGE;

extern PLAUNCH_DATA_PAGE *LaunchDataPage;

#ifdef __cplusplus
}
#endif

#endif // _XLAUNCH_H_
