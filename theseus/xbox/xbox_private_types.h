#ifndef _XBOX_PRIVATE_TYPES_H
#define _XBOX_PRIVATE_TYPES_H

// Basic Win32 Types (required when included before standard headers)
#ifndef _WINDEF_
typedef long LONG;
typedef unsigned short USHORT;
typedef char CHAR;
typedef char* PCHAR;
typedef unsigned long ULONG;
typedef unsigned long* PULONG;
typedef void* PVOID;
typedef void* HANDLE;
typedef HANDLE* PHANDLE;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef int BOOL;
typedef wchar_t WCHAR;
typedef WCHAR* PWSTR;
typedef const WCHAR* LPCWSTR;
typedef int BOOLEAN;
typedef void VOID;
typedef long long LONGLONG;
typedef unsigned long ULONG_PTR;
typedef short CSHORT;
typedef unsigned char UCHAR;
typedef unsigned char* PUCHAR;
typedef long HRESULT;

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _LUID {
    DWORD LowPart;
    LONG HighPart;
} LUID, *PLUID;

#ifndef WINAPI
#define WINAPI __stdcall
#endif
#ifndef NTAPI
#define NTAPI __stdcall
#endif
#ifndef DECLSPEC_NORETURN
#define DECLSPEC_NORETURN __declspec(noreturn)
#endif
#endif

// NT string pointer that windef.h doesn't provide, so it lives outside the
// _WINDEF_ guard above.
typedef const char* PCSZ;

typedef struct _SYSTEMTIME SYSTEMTIME, *PSYSTEMTIME;

// SMC Tray States. The state lives in bits 4-6 (mask 0x70); bit 0 is an
// activity flag. These are the real SMC values, an empty tray reads NO_MEDIA.
#define SMC_TRAY_STATE_ACTIVITY         0x01
#define SMC_TRAY_STATE_STATE_MASK       0x70
#define SMC_TRAY_STATE_CLOSED           0x00
#define SMC_TRAY_STATE_OPEN             0x10
#define SMC_TRAY_STATE_UNLOADING        0x20
#define SMC_TRAY_STATE_OPENING          0x30
#define SMC_TRAY_STATE_NO_MEDIA         0x40
#define SMC_TRAY_STATE_CLOSING          0x50
#define SMC_TRAY_STATE_MEDIA_DETECT     0x60
#define SMC_TRAY_STATE_RESET            0x70

// CD-ROM IOCTL Check Verify
#define IOCTL_CDROM_CHECK_VERIFY        0x00024800

typedef struct _XPP_DEVICE_TYPE XPP_DEVICE_TYPE, *PXPP_DEVICE_TYPE;


// CD-ROM structures and IOCTL constants
#define IOCTL_CDROM_READ_TOC            0x00024000
#define IOCTL_CDROM_RAW_READ            0x0002403E

#define TOC_DATA_TRACK                  0x04
#define TOC_LAST_TRACK                  0xAA

typedef struct _TRACK_DATA {
    BYTE Reserved;
    BYTE Control : 4;
    BYTE Adr : 4;
    BYTE TrackNumber;
    BYTE Reserved1;
    BYTE Address[4];
} TRACK_DATA, *PTRACK_DATA;

typedef struct _CDROM_TOC {
    BYTE Length[2];
    BYTE FirstTrack;
    BYTE LastTrack;
    TRACK_DATA TrackData[100];
} CDROM_TOC, *PCDROM_TOC;

typedef enum _TRACK_MODE_TYPE {
    YellowMode2,
    XAForm2,
    CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;

typedef struct __RAW_READ_INFO {
    LARGE_INTEGER DiskOffset;
    ULONG SectorCount;
    TRACK_MODE_TYPE TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;

// =========================================================================
// NT Kernel Types (typically from ntos.h / ntrtl.h)
// =========================================================================

typedef LONG NTSTATUS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_NO_MORE_FILES             ((NTSTATUS)0x80000006L)
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING, ANSI_STRING, *PANSI_STRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    HANDLE RootDirectory;
    PANSI_STRING ObjectName;
    ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define OBJ_CASE_INSENSITIVE 0x00000040L

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->RootDirectory = r;                             \
    (p)->ObjectName = n;                                \
    (p)->Attributes = a;                                \
    }

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } Pointer;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// =========================================================================
// Directory Search / Filesystem structs & flags
// =========================================================================

#define FILE_SHARE_READ                 0x00000001
#define FILE_SHARE_WRITE                0x00000002
#define FILE_SHARE_DELETE               0x00000004

#define FILE_DIRECTORY_FILE             0x00000001
#define FILE_WRITE_THROUGH              0x00000002
#define FILE_SEQUENTIAL_ONLY            0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING  0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT       0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT    0x00000020
#define FILE_NON_DIRECTORY_FILE         0x00000040
#define FILE_CREATE_TREE_CONNECTION     0x00000080
#define FILE_COMPLETE_IF_OPLOCKED       0x00000100
#define FILE_NO_EA_KNOWLEDGE            0x00000200
#define FILE_OPEN_FOR_RECOVERY          0x00000400
#define FILE_RANDOM_ACCESS              0x00000800
#define FILE_DELETE_ON_CLOSE            0x00001000
#define FILE_OPEN_BY_FILE_ID            0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT     0x00004000
#define FILE_NO_COMPRESSION             0x00008000

#define FSCTL_WRITE_VOLUME_METADATA     0x00090018

typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    CHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

// =========================================================================
// Timezone and EEPROM configuration
// =========================================================================

#ifndef REG_BINARY
#define REG_BINARY                      3
#endif
#ifndef REG_DWORD
#define REG_DWORD                       4
#endif

#define XC_MISC_FLAGS                   0x0107
#define XC_MISC_FLAG_AUTOPOWERDOWN      0x00000001
#define XC_MAX_OS                       0x000010BB

typedef struct _XBOX_TIMEZONE_DATE {
    BYTE Month;
    BYTE Day;
    BYTE DayOfWeek;
    BYTE Hour;
} XBOX_TIMEZONE_DATE;

typedef struct _XBOX_USER_SETTINGS {
    BYTE  SettingsVersion;
    BYTE  TimeZoneBias;
    BYTE  TimeZoneStdName[4];
    BYTE  TimeZoneDltName[4];
    XBOX_TIMEZONE_DATE TimeZoneStdDate;
    XBOX_TIMEZONE_DATE TimeZoneDltDate;
    BYTE  TimeZoneStdBias;
    BYTE  TimeZoneDltBias;
    // Padding for standard settings structure size (typically 256 bytes in EEPROM)
    BYTE  Padding[236]; 
} XBOX_USER_SETTINGS;

typedef struct _XBOX_REFURB_INFO {
    LARGE_INTEGER FirstSetTime;
    LARGE_INTEGER LastSetTime;
} XBOX_REFURB_INFO;

#define XPROF_START                     0
#define XPROF_STOP                      1

typedef struct _KDPC {
    BYTE Header[32];
} KDPC, *PKDPC;

typedef struct _KTIMER {
    BYTE Header[40];
} KTIMER, *PKTIMER;

// Simple inline timezone name compressor used by settings.
static __inline VOID WstrToXboxTimeZoneName(LPCWSTR lpName, BYTE out[4]) {
    int i;
    for (i = 0; i < 4; i++) {
        out[i] = (lpName && lpName[i]) ? (BYTE)(lpName[i] & 0xFF) : 0;
    }
}

// =========================================================================
// Encryption & Kernel Private Exports
// =========================================================================

typedef enum _FIRMWARE_REBOOT_TYPE {
    HalHaltRoutine,
    HalRebootRoutine,
    HalQuickRebootRoutine,
    HalKdRebootRoutine,
    HalFatalErrorRebootRoutine,
    HalMaxReentryRoutine
} FIRMWARE_REBOOT_TYPE, FIRMWARE_REENTRY;

#ifdef __cplusplus
extern "C" {
#endif
    // NT Kernel Functions
    VOID NTAPI RtlInitAnsiString(PANSI_STRING DestinationString, const char* SourceString);
    ULONG NTAPI RtlNtStatusToDosError(NTSTATUS Status);

    NTSTATUS NTAPI NtOpenFile(
        OUT PHANDLE FileHandle,
        IN ACCESS_MASK DesiredAccess,
        IN POBJECT_ATTRIBUTES ObjectAttributes,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN ULONG ShareAccess,
        IN ULONG OpenOptions
    );

    NTSTATUS NTAPI NtReadFile(
        IN HANDLE FileHandle,
        IN HANDLE Event OPTIONAL,
        IN PVOID ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        OUT PVOID Buffer,
        IN ULONG Length,
        IN PLARGE_INTEGER ByteOffset OPTIONAL
    );

    NTSTATUS NTAPI NtWriteFile(
        IN HANDLE FileHandle,
        IN HANDLE Event OPTIONAL,
        IN PVOID ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN PVOID Buffer,
        IN ULONG Length,
        IN PLARGE_INTEGER ByteOffset OPTIONAL
    );

    NTSTATUS NTAPI NtClose(IN HANDLE Handle);

    NTSTATUS NTAPI NtFsControlFile(
        IN HANDLE FileHandle,
        IN HANDLE Event OPTIONAL,
        IN PVOID ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN ULONG FsControlCode,
        IN PVOID InputBuffer OPTIONAL,
        IN ULONG InputBufferLength,
        OUT PVOID OutputBuffer OPTIONAL,
        IN ULONG OutputBufferLength
    );

    typedef int FILE_INFORMATION_CLASS;
    #define FileDirectoryInformation 1
    #define FileBasicInformation 4
    #define FileDispositionInformation 13
    #define FILE_ATTRIBUTE_VALID_SET_FLAGS 0x000037a7

    NTSTATUS NTAPI NtQueryDirectoryFile(
        IN HANDLE FileHandle,
        IN HANDLE Event OPTIONAL,
        IN PVOID ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        OUT PVOID FileInformation,
        IN ULONG Length,
        IN FILE_INFORMATION_CLASS FileInformationClass,
        IN PSTRING FileName OPTIONAL,
        IN BOOLEAN RestartScan
    );

    NTSTATUS NTAPI NtSetInformationFile(
        IN HANDLE FileHandle,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN PVOID FileInformation,
        IN ULONG Length,
        IN FILE_INFORMATION_CLASS FileInformationClass
    );

    typedef struct _TIME_FIELDS {
        short Year;
        short Month;
        short Day;
        short Hour;
        short Minute;
        short Second;
        short Milliseconds;
        short Weekday;
    } TIME_FIELDS, *PTIME_FIELDS;

    BOOLEAN NTAPI RtlTimeFieldsToTime(PTIME_FIELDS TimeFields, PLARGE_INTEGER Time);
    NTSTATUS NTAPI NtSetSystemTime(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER PreviousTime OPTIONAL);
    
    // Hal/Ex Functions (SMC, EEPROM, and Reboot)
    extern ULONG* HalDiskCachePartitionCount;
    extern LPBYTE *XboxHDKey;

    VOID NTAPI HalReturnToFirmware(FIRMWARE_REBOOT_TYPE RebootType);

    // Contiguous physical memory. The launch data page is allocated here and
    // marked persistent so it survives a warm reboot into the next title.
    PVOID NTAPI MmAllocateContiguousMemory(ULONG NumberOfBytes);
    VOID  NTAPI MmPersistContiguousMemory(PVOID BaseAddress, ULONG NumberOfBytes, BOOLEAN Persist);

    NTSTATUS NTAPI HalReadSMBusValue(
        IN UCHAR Address,
        IN UCHAR Command,
        IN BOOLEAN ReadWord,
        OUT PULONG Value
    );
    
    NTSTATUS NTAPI HalWriteSMBusValue(
        IN UCHAR Address,
        IN UCHAR Command,
        IN BOOLEAN WriteWord,
        IN ULONG Value
    );

    NTSTATUS NTAPI HalReadSMCTrayState(
        OUT PDWORD pTrayState,
        OUT PDWORD pTrayState2 OPTIONAL
    );

    NTSTATUS NTAPI ExSaveNonVolatileSetting(
        IN ULONG ValueIndex,
        IN ULONG Type,
        IN PVOID Value,
        IN ULONG ValueLength
    );
    
    NTSTATUS NTAPI ExReadWriteRefurbInfo(
        IN PVOID Buffer,
        IN ULONG BufferLength,
        IN BOOLEAN Write
    );

    // Ke DPC & Timer API
    typedef VOID (NTAPI *PKDEFERRED_ROUTINE)(
        IN struct _KDPC *Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2
    );

    VOID NTAPI KeInitializeDpc(
        IN PKDPC Dpc,
        IN PKDEFERRED_ROUTINE DeferredRoutine,
        IN PVOID DeferredContext
    );

    typedef enum _TIMER_TYPE {
        NotificationTimer,
        SynchronizationTimer
    } TIMER_TYPE;

    VOID NTAPI KeInitializeTimerEx(
        IN PKTIMER Timer,
        IN TIMER_TYPE Type
    );

    // Plain KeInitializeTimer is just the notification timer flavour. The
    // kernel only exports the Ex form, so this is a macro, same as the SDK.
    #define KeInitializeTimer(Timer) KeInitializeTimerEx(Timer, NotificationTimer)

    BOOLEAN NTAPI KeSetTimer(
        IN PKTIMER Timer,
        IN LARGE_INTEGER DueTime,
        IN PKDPC Dpc OPTIONAL
    );

    BOOLEAN NTAPI KeCancelTimer(
        IN PKTIMER Timer
    );

    // XAPI / Private SDK functions
    DWORD WINAPI XQueryValue(
        IN ULONG ValueIndex,
        OUT PULONG Type,
        OUT PVOID Value,
        IN ULONG ValueLength,
        OUT PULONG ValueLengthReturned
    );

    DWORD WINAPI XSetValue(
        IN ULONG ValueIndex,
        IN ULONG Type,
        IN PVOID Value,
        IN ULONG ValueLength
    );



    VOID NTAPI XProfpControl(ULONG ControlCode, ULONG Parameter);

    BOOL WINAPI XapiSetLocalTime(const SYSTEMTIME *lpLocalTime);
    VOID WINAPI XAutoPowerDownResetTimer(VOID);

    extern PVOID XeImageFileName;
    VOID WINAPI XapiDeleteCachePartition(DWORD TitleID);
    ULONG _cdecl DbgPrint(const char* Format, ...);

    #ifndef LDT_NONE
    #define LDT_NONE 0xFFFFFFFF
    #endif
    // The SDK lib exports the ANSI XWriteTitleInfoAndRebootA; the plain name
    // is the usual name mapping macro over it.
    VOID WINAPI XWriteTitleInfoAndRebootA(
        IN LPCSTR pszLaunchPath,
        IN LPCSTR pszDDPath,
        IN DWORD dwLaunchDataType,
        IN DWORD dwTitleId,
        IN PVOID pLaunchData
    );
    #define XWriteTitleInfoAndReboot XWriteTitleInfoAndRebootA

    // The SDK lib exports the ANSI XMountMURootA; XMountMURoot is the usual
    // name mapping macro over it.
    DWORD WINAPI XMountMURootA(DWORD dwPort, DWORD dwSlot, CHAR* pchDrive);
    #define XMountMURoot XMountMURootA
    DWORD WINAPI XCleanMUFromRoot(CHAR chDrive, LPCSTR pszPreserveDir);
    DWORD WINAPI XMUWriteNameToDriveLetter(CHAR chDrive, LPCWSTR lpName);

    typedef STRING OBJECT_STRING;
    typedef PSTRING POBJECT_STRING;
    #define CONSTANT_OBJECT_STRING(s) { sizeof(s) - 1, sizeof(s) - 1, (PSTR)(s) }

    typedef struct _XBOX_KRNL_VERSION {
        USHORT Major;
        USHORT Minor;
        USHORT Build;
        USHORT Qfe;
    } XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;
    extern PXBOX_KRNL_VERSION XboxKrnlVersion;

    VOID WINAPI MU_CloseDeviceObject(DWORD dwPort, DWORD dwSlot);

    typedef VOID (NTAPI *PIO_APC_ROUTINE)(
        IN PVOID ApcContext,
        IN PIO_STATUS_BLOCK IoStatusBlock,
        IN ULONG Reserved
    );

    NTSTATUS NTAPI NtAllocateVirtualMemory(
        IN OUT PVOID *BaseAddress,
        IN ULONG_PTR ZeroBits,
        IN OUT PSIZE_T RegionSize,
        IN ULONG AllocationType,
        IN ULONG Protect
    );

    NTSTATUS NTAPI NtFreeVirtualMemory(
        IN OUT PVOID *BaseAddress,
        IN OUT PSIZE_T RegionSize,
        IN ULONG FreeType
    );

    NTSTATUS NTAPI NtDeviceIoControlFile(
        IN HANDLE FileHandle,
        IN HANDLE Event OPTIONAL,
        IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
        IN PVOID ApcContext OPTIONAL,
        OUT PIO_STATUS_BLOCK IoStatusBlock,
        IN ULONG IoControlCode,
        IN PVOID InputBuffer OPTIONAL,
        IN ULONG InputBufferLength,
        OUT PVOID OutputBuffer OPTIONAL,
        IN ULONG OutputBufferLength
    );

    NTSTATUS NTAPI IoDismountVolumeByName(
        IN POBJECT_STRING VolumeName
    );

    NTSTATUS NTAPI IoCreateSymbolicLink(
        IN POBJECT_STRING SymbolicLinkName,
        IN POBJECT_STRING DeviceName
    );

    NTSTATUS NTAPI IoDeleteSymbolicLink(
        IN POBJECT_STRING SymbolicLinkName
    );

    VOID NTAPI MmFreeContiguousMemory(IN PVOID BaseAddress);

    #define XC_VIDEO_FLAGS 3

    typedef struct _XBOX_HARDWARE_INFO {
        ULONG Flags;
        UCHAR GpuRevision;
        UCHAR McpxRevision;
        UCHAR HwRevision;
        UCHAR Reserved[1];
    } XBOX_HARDWARE_INFO, *PXBOX_HARDWARE_INFO;
    extern PXBOX_HARDWARE_INFO XboxHardwareInfo;

    #define XBOX_480P_MACROVISION_ENABLED 0x00000002

    DWORD WINAPI MU_CreateDeviceObject(DWORD dwPort, DWORD dwSlot, PVOID pDeviceObject);
    BOOL WINAPI XapiFormatFATVolume(PVOID pDeviceObject);

    #define RtlInitObjectString RtlInitAnsiString

    // AV Aspect Ratio & Video flags from private av.h
    #define AV_FLAGS_WIDESCREEN               0x00010000
    #define AV_FLAGS_LETTERBOX                0x00100000
    #define AV_FLAGS_HDTV_720p                0x00020000
    #define AV_FLAGS_HDTV_1080i               0x00040000
    #define AV_FLAGS_HDTV_480p                0x00080000
    #define AV_FLAGS_60Hz                     0x00400000

    #define XC_FACTORY_STARTUP_LANE                 1
    #define XC_USER_STARTUP_LANE                    2
    #define XC_AUDIO_FLAGS                          4
    #define XC_LANGUAGE                             5
    #define XC_PARENTAL_CONTROL_GAMES               6
    #define XC_PARENTAL_CONTROL_PASSWORD            7
    #define XC_PARENTAL_CONTROL_MOVIES              8
    #define XC_ONLINE_IP_ADDRESS                    9
    #define XC_ONLINE_DNS_ADDRESS                   10
    #define XC_ONLINE_DEFAULT_GATEWAY_ADDRESS       11
    #define XC_ONLINE_SUBNET_MASK                   12
    #ifndef XC_MISC_FLAGS
    #define XC_MISC_FLAGS                           13
    #endif
    #define XC_DVD_REGION                           14

    #define XC_MISC_FLAG_DONT_USE_DST               0x00000002
    #define XC_VIDEO_STANDARD_PAL_M                 4

    DWORD WINAPI XAutoPowerDownGet(BOOL* pfAutoPowerDown);
    DWORD WINAPI XAutoPowerDownSet(BOOL fAutoPowerDown);

    VOID NTAPI HalInitiateShutdown(VOID);

    typedef struct _TIME_ZONE_INFORMATION TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;

    DWORD WINAPI XapipQueryTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation, PBOOL pbUseDST);
    DWORD WINAPI XapipSetTimeZoneInformation(PTIME_ZONE_INFORMATION lpTimeZoneInformation);
    typedef char KPROCESSOR_MODE;

    NTSTATUS NTAPI KeDelayExecutionThread(
        IN KPROCESSOR_MODE WaitMode,
        IN BOOLEAN Alertable,
        IN PLARGE_INTEGER Interval
    );
#ifdef __cplusplus
}
#endif

#endif // _XBOX_PRIVATE_TYPES_H

#if defined(_XBOX_) && !defined(_XBOX_INPUT_TYPES_H_)
#define _XBOX_INPUT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// Input / IR remote private definitions (only defined after Xbox.h has defined XINPUT_GAMEPAD)
typedef struct _XINPUT_IR_REMOTE {
    WORD wKeyCode;
    WORD wTimeDelta;
} XINPUT_IR_REMOTE, *PXINPUT_IR_REMOTE;

typedef struct _XINPUT_STATE_INTERNAL {
    DWORD dwPacketNumber;
    union {
        XINPUT_GAMEPAD Gamepad;
        XINPUT_IR_REMOTE IrRemote;
    };
} XINPUT_STATE_INTERNAL, *PXINPUT_STATE_INTERNAL;

// The real IR remote device type table lives in the SDK lib. Point at it, the
// input driver reads this table so a placeholder would fault.
extern XPP_DEVICE_TYPE XDEVICE_TYPE_IR_REMOTE_TABLE;
#define XDEVICE_TYPE_IR_REMOTE (&XDEVICE_TYPE_IR_REMOTE_TABLE)


#ifdef __cplusplus
}
#endif

#endif // _XBOX_INPUT_TYPES_H_
