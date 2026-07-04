#ifndef _XONLINEP_H
#define _XONLINEP_H

// The dashboard needs the public XOnline surface (XUID, startup, the stored
// user count) plus one private helper that reads the accounts cached on the
// hard drive without a live logon.

#include <xonline.h>

#ifdef __cplusplus
extern "C" {
#endif

// The account records the kernel keeps on the hard drive are wider than the
// public XONLINE_USER. They carry a kingdom, a pin, and a trailing index that
// the public view drops, so the whole thing is 128 bytes at pack 4. This is
// the layout _XOnlineGetUsersFromHD actually fills. Hand it an array of the
// public struct and every record spills 16 bytes past its slot, which is how
// you smash whatever global sits after the array.
#pragma pack(push, 4)
typedef struct {
	ULONGLONG qwUserID;
	DWORD     dwUserFlags;
} XUID_HD;

typedef struct {
	XUID_HD xuid;
	CHAR    name[16];
	CHAR    kingdom[12];
	DWORD   dwUserOptions;
	BYTE    pin[4];
	BYTE    reserved[72];
	HRESULT hr;
	DWORD   index;
} XONLINE_USER_HD;
#pragma pack(pop)

HRESULT WINAPI _XOnlineGetUsersFromHD(XONLINE_USER_HD *pUsers, DWORD *pcUsers);

#ifdef __cplusplus
}
#endif

#endif // _XONLINEP_H
