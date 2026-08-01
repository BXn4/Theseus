// xdk_compat.h -- Compatibility shims for building Xbox XDK code with clang

#ifndef _XDK_COMPAT_H
#define _XDK_COMPAT_H

// excpt.h defines EXCEPTION_DISPOSITION which winnt.h needs.
#include <Excpt.h>

// Include the public Win32 type definitions first so we have LRESULT, HWND, etc.
#include <windef.h>
#include <winnt.h>

// Now include our clean compatibility header to define NT kernel and private exports
#include "../theseus/xbox/xbox_private_types.h"

// Tell winbase.h that NT kernel types are already defined (prevents redefinition conflicts)
#define _NTOS_

// Since we define _NTOS_, winbase.h skips the Interlocked function declarations.
// We map them directly to Clang's compiler builtins here so they compile cleanly.
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedDecrement       _InterlockedDecrement
#define InterlockedExchange        _InterlockedExchange
#define InterlockedExchangeAdd     _InterlockedExchangeAdd
#define InterlockedIncrement       _InterlockedIncrement

// Tell toolbox/xboxinternals.h that all kernel types are available.
#define _THESEUS_STD_H

#endif
