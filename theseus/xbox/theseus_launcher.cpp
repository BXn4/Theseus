// theseus_launcher.cpp: CTheseusLauncher XAP node. Script-callable
// title launcher. Supports XBE paths, .uixshortcut files, and
// virtual entries from games.ini (VGames). Theseus-original.

#include "theseus_launcher.h"

IMPLEMENT_NODE("TheseusLauncher", CTheseusLauncher, CNode)
#define _FND_CLASS CTheseusLauncher
START_NODE_FUN(CTheseusLauncher, CNode)
NODE_FUN_VS(Launch)
END_NODE_FUN()
#undef _FND_CLASS

CTheseusLauncher::CTheseusLauncher()
{
}

CTheseusLauncher::~CTheseusLauncher()
{
}
void CTheseusLauncher::Launch(const TCHAR *szPath)
{
    char szAnsi[MAX_PATH];

#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, szPath, -1, szAnsi, MAX_PATH, NULL, NULL);
#else
    strncpy(szAnsi, szPath, MAX_PATH);
    szAnsi[MAX_PATH - 1] = '\0';
#endif

    // Print debug message with full path
    char szDebug[MAX_PATH + 64];
    sprintf(szDebug, "TheseusLauncher: Requested launch of [%s]\n", szAnsi);
    OutputDebugStringA(szDebug);

    if (EndsWith(szAnsi, ".xbe"))
    {
        LaunchXBE(szAnsi);
        return;
    }

    if (EndsWith(szAnsi, ".iso") || EndsWith(szAnsi, ".cci"))
    {
        LaunchWithAttach(szAnsi);
        return;
    }

    OutputDebugStringA("TheseusLauncher: Unsupported file type.\n");
}

bool CTheseusLauncher::LaunchXBE(const char *szPath)
{
    OutputDebugString(L"TheseusLauncher: Launching XBE...\n");
    return XLaunchNewImage(szPath, NULL) == S_OK;
}

// Cerbios wants a raw NT device path for the slice file. A drive letter like
// "F:\" won't resolve inside the virtual driver, so map it to a device path
// with the same drive table the overlay uses.
static bool DosToNtPath(const char *dosPath, char *ntPath, int ntPathSize)
{
    if (!dosPath || strlen(dosPath) < 3 || dosPath[1] != ':' || dosPath[2] != '\\')
        return false;

    struct { char letter; const char *ntDev; } driveMap[] = {
        {'C', "\\Device\\Harddisk0\\partition2"},
        {'E', "\\Device\\Harddisk0\\partition1"},
        {'F', "\\Device\\Harddisk0\\partition6"},
        {'G', "\\Device\\Harddisk0\\partition7"},
        {'R', "\\Device\\Harddisk0\\partition8"},
        {'S', "\\Device\\Harddisk0\\partition9"},
        {'X', "\\Device\\Harddisk0\\partition5"},
        {'Y', "\\Device\\Harddisk0\\partition4"},
        {'Z', "\\Device\\Harddisk0\\partition3"},
    };

    char letter = dosPath[0];
    if (letter >= 'a' && letter <= 'z')
        letter -= 32;

    const char *ntDev = NULL;
    for (int i = 0; i < (int)(sizeof(driveMap) / sizeof(driveMap[0])); ++i)
        if (driveMap[i].letter == letter) { ntDev = driveMap[i].ntDev; break; }

    if (!ntDev)
        return false;

    const char *remainder = dosPath + 3;
    if (*remainder)
        _snprintf(ntPath, ntPathSize, "%s\\%s", ntDev, remainder);
    else
        _snprintf(ntPath, ntPathSize, "%s", ntDev);
    return true;
}

bool CTheseusLauncher::LaunchWithAttach(const char *szISOPath)
{
    const bool isCCI = EndsWith(szISOPath, ".cci");
    const USHORT build = XboxKrnlVersion->Build;

    // Turn a drive letter path into a device path. If it already is one, keep it.
    char ntPath[MAX_PATH];
    const char *path = szISOPath;
    if (DosToNtPath(szISOPath, ntPath, sizeof(ntPath)))
        path = ntPath;

    const char *file = strrchr(path, '\\');
    if (file)
        file++;
    else
        file = path;

    if (build >= 8008)
    {

        OutputDebugString(L"TheseusLauncher: Detected Cerbios\n");
        return AttachCerbios(path, file, isCCI);
    }
    else
    {
        OutputDebugString(L"TheseusLauncher: Legacy BIOS detected\n");
        return AttachLegacy(path, file, build);
    }
}

bool CTheseusLauncher::AttachCerbios(const char *path, const char *file, bool isCCI)
{
    void *membuf = NULL;
    ULONG membuf_size = 1024 * 1024;
    if (!NT_SUCCESS(NtAllocateVirtualMemory(&membuf, 0, &membuf_size, MEM_COMMIT | MEM_NOZERO, PAGE_READWRITE)))
        return false;

    ATTACH_SLICE_DATA_CERBIOS *asd = (ATTACH_SLICE_DATA_CERBIOS *)membuf;
    memset(asd, 0, sizeof(ATTACH_SLICE_DATA_CERBIOS));
    asd->DeviceType = isCCI ? 'd' : 'D';

    char *strbuf = (char *)membuf + sizeof(ATTACH_SLICE_DATA_CERBIOS);
    membuf_size -= sizeof(ATTACH_SLICE_DATA_CERBIOS);

    // Mount Point is always \Device\CdRom0
    _snprintf(strbuf, membuf_size, "\\Device\\CdRom0");
    RtlInitAnsiString(&asd->MountPoint, strbuf);
    asd->MountPoint.MaximumLength = asd->MountPoint.Length + 1;
    strbuf += asd->MountPoint.MaximumLength;
    membuf_size -= asd->MountPoint.MaximumLength;

    // Correct full path to ISO/CCI file
    _snprintf(strbuf, membuf_size, "%s", path);
    RtlInitAnsiString(&asd->SliceFile[0], strbuf);
    asd->SliceFile[0].MaximumLength = asd->SliceFile[0].Length + 1;
    asd->SliceCount = 1;

    // Debug
    OutputDebugStringA("TheseusLauncher: Cerbios full file path: ");
    OutputDebugStringA(strbuf);
    OutputDebugStringA("\r\n");

    // Open virtual device
    ANSI_STRING dev_name;
    RtlInitAnsiString(&dev_name, "\\Device\\Virtual0\\Image0");
    OBJECT_ATTRIBUTES obj_attr;
    InitializeObjectAttributes(&obj_attr, &dev_name, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK io_status;
    HANDLE h;
    if (!NT_SUCCESS(NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status,
                               FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE)))
        return false;

    // Detach
    NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_DETACH, NULL, 0, NULL, 0);

    // Attach
    bool success = NT_SUCCESS(NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status,
                                                    IOCTL_VIRTUAL_ATTACH, asd, sizeof(ATTACH_SLICE_DATA_CERBIOS), NULL, 0));
    NtClose(h);

    if (success)
    {
        // Hard quick reboot into the freshly mounted disc, matching Hermes.
        // Cerbios keeps the CCI decompression state alive across the reboot,
        // so the BIOS detects and boots it.
        OutputDebugString(L"TheseusLauncher: Cerbios attach OK, rebooting to mounted disc\n");
        HalReturnToFirmware(HalQuickRebootRoutine);
    }

    return false;
}

bool CTheseusLauncher::AttachLegacy(const char *path, const char *file, USHORT build)
{
    void *membuf = NULL;
    ULONG membuf_size = 1024 * 1024;
    if (!NT_SUCCESS(NtAllocateVirtualMemory(&membuf, 0, &membuf_size, MEM_COMMIT | MEM_NOZERO, PAGE_READWRITE)))
        return false;

    ATTACH_SLICE_DATA_LEGACY *asd = (ATTACH_SLICE_DATA_LEGACY *)membuf;
    memset(asd, 0, sizeof(ATTACH_SLICE_DATA_LEGACY));

    char *strbuf = (char *)membuf + sizeof(ATTACH_SLICE_DATA_LEGACY);
    membuf_size -= sizeof(ATTACH_SLICE_DATA_LEGACY);

    _snprintf(strbuf, membuf_size, "%s", path);
    RtlInitAnsiString(&asd->SliceFile[0], strbuf);
    asd->SliceFile[0].MaximumLength = asd->SliceFile[0].Length + 1;
    asd->SliceCount = 1;

    ANSI_STRING dev_name;
    RtlInitAnsiString(&dev_name, "\\Device\\CdRom1");
    OBJECT_ATTRIBUTES obj_attr;
    InitializeObjectAttributes(&obj_attr, &dev_name, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK io_status;
    HANDLE h;
    if (!NT_SUCCESS(NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE)))
        return false;

    NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_CDROM_DETACH, NULL, 0, NULL, 0);
    NtClose(h);

    IoDismountVolumeByName(&dev_name);
    if (!NT_SUCCESS(NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT)))
        return false;

    bool success = NT_SUCCESS(NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_CDROM_ATTACH, asd, sizeof(ATTACH_SLICE_DATA_LEGACY), NULL, 0));

    if (build == 5003 || build == 5004)
    {
        STRING saMountPoint, saSystemPath;
        RtlInitAnsiString(&saMountPoint, "\\??\\D:");
        RtlInitAnsiString(&saSystemPath, "\\Device\\CdRom0");
        IoDeleteSymbolicLink(&saMountPoint);
        IoCreateSymbolicLink(&saMountPoint, &saSystemPath);
        XLaunchNewImage("D:\\default.xbe", NULL);
    }
    else if (success)
    {
        // Soft launch into the mounted virtual disc, matching the overlay's
        // working flow. On this build a hard reboot drops the CCI mount, so
        // we hand off with XLaunchNewImage to keep it intact.
        OutputDebugString(L"TheseusLauncher: Legacy attach OK, launching D:\\default.xbe\n");
        XLaunchNewImage("D:\\default.xbe", NULL);
    }
    NtClose(h);
    return success;
}

bool CTheseusLauncher::FileExists(const char *szPath)
{
    ANSI_STRING path;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK ioStatus;
    HANDLE hFile;

    RtlInitAnsiString(&path, szPath);
    InitializeObjectAttributes(&objAttr, &path, OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS status = NtOpenFile(&hFile, GENERIC_READ | SYNCHRONIZE, &objAttr, &ioStatus, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status))
        return false;

    NtClose(hFile);
    return true;
}

bool CTheseusLauncher::EndsWith(const char *str, const char *suffix)
{
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return false;
    return _stricmp(str + lenstr - lensuffix, suffix) == 0;
}

// Free-function entry for callers that can't pull in theseus_launcher.h
// without header-include conflicts (overlay.cpp's file-manager launch
// path). Spins up a transient CTheseusLauncher and dispatches.
extern "C" void LaunchFileFromOverlay(const TCHAR *szPath)
{
    CTheseusLauncher launcher;
    launcher.Launch(szPath);
}
