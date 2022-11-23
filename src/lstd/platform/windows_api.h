#pragma once

//
// In this file we define some types found in Windows.h.
// We do this in order to avoid including Windows.h at all.
// This makes the program compile faster, but that's not the actual reason we do this...
// In reality Windows.h complains about not having symbols normally found in the CRT!
// :AvoidSTL: We avoid including any files from the C++ standard library.
//

using HRESULT = u32;

#define FACILITY_WIN32 7

#define S_OK 0x00000000
#define S_FALSE 0x00000001
#define E_NOTIMPL 0x80004001
#define E_NOINTERFACE 0x80004002
#define E_POINTER 0x80004003
#define E_ABORT 0x80004004
#define E_FAIL 0x80004005
#define E_UNEXPECTED 0x8000FFFF
#define DISP_E_MEMBERNOTFOUND 0x80020003
#define DISP_E_TYPEMISMATCH 0x80020005
#define DISP_E_UNKNOWNNAME 0x80020006
#define DISP_E_EXCEPTION 0x80020009
#define DISP_E_OVERFLOW 0x8002000A
#define DISP_E_BADINDEX 0x8002000B
#define DISP_E_BADPARAMCOUNT 0x8002000E
#define DISP_E_PARAMNOTOPTIONAL 0x8002000F
#define SCRIPT_E_REPORTED 0x80020101
#define STG_E_INVALIDFUNCTION 0x80030001
#define DESTS_E_NO_MATCHING_ASSOC_HANDLER 0x80040F03
#define E_ACCESSDENIED 0x80070005
#define E_OUTOFMEMORY 0x8007000E
#define E_INVALIDARG 0x80070057
#define COR_E_OBJECTDISPOSED 0x80131622
#define WC_E_GREATERTHAN 0xC00CEE23
#define WC_E_SYNTAX 0xC00CEE2D

//
// HRESULT_FROM_WIN32(x) used to be a macro, however we now run it as an inline function
// to prevent double evaluation of 'x'. If you still need the macro, you can use __HRESULT_FROM_WIN32(x)
//
#define __HRESULT_FROM_WIN32(x) ((HRESULT) (x) <= 0 ? ((HRESULT) (x)) : ((HRESULT) (((x) &0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))
always_inline HRESULT HRESULT_FROM_WIN32(unsigned long x) {
    return (HRESULT) x <= 0 ? (HRESULT) x : (HRESULT) (x & 0x0000FFFF | FACILITY_WIN32 << 16 | 0x80000000);
}

#define MAX_PATH 260

#define CP_UTF8 65001

#define INVALID_HANDLE_VALUE ((void *) (-1))

using BYTE  = u8;
using PBYTE = u8 *;

using INT8  = s8;
using UINT8 = u8;

using INT16  = s16;
using UINT16 = u16;

using SHORT  = s16;
using USHORT = u16;

using ATOM = u16;
using WORD = u16;

using DWORD   = u32;
using PDWORD  = u32 *;
using LPDWORD = u32 *;

using DWORD64  = u64;
using PDWORD64 = u64 *;

using DWORD_PTR = u64;

using UINT  = u32;
using PUINT = UINT *;

using INT64  = s64;
using UINT64 = u64;

using ULONG     = u32;
using ULONG32   = u32;
using ULONG_PTR = s64;

using ULONG64 = u64;

using LONGLONG  = s64;
using ULONGLONG = u64;

using SIZE_T  = s64;
using PSIZE_T = s64 *;

using UINT_PTR = u64;

using LONG     = s32;
using LONG_PTR = s64;

using LRESULT = s64;

using LPARAM = LONG_PTR;
using WPARAM = UINT_PTR;

using WCHAR   = wchar;
using LPCCH   = const char *;
using LPSTR   = const char *;
using PCSTR   = const char *;
using LPCSTR  = const char *;
using PWSTR   = wchar *;
using LPWSTR  = wchar *;
using LPCWSTR = const wchar *;
using LPCWCH  = const wchar *;

using PVOID   = void *;
using LPVOID  = void *;
using LPCVOID = const void *;

using VOID = void;

#define CONST const

using CHAR  = char;
using PCHAR = char *;

using BOOL    = s32;
using BOOLEAN = byte;

using LPBOOL = BOOL *;

using HANDLE                = void *;
using HWND                  = HANDLE;
using HMODULE               = HANDLE;
using HDEVNOTIFY            = HANDLE;
using HLOCAL                = HANDLE;
using HGLOBAL               = HANDLE;
using HMENU                 = HANDLE;
using HRGN                  = HANDLE;
using HMONITOR              = HANDLE;
using DPI_AWARENESS_CONTEXT = HANDLE;
using HDC                   = HANDLE;
using HGDIOBJ               = HANDLE;
using HBITMAP               = HANDLE;
using HRAWINPUT             = HANDLE;
using HDROP                 = HANDLE;

using COLORREF   = DWORD;
using LPCOLORREF = DWORD *;

using FARPROC = void *;

typedef struct _OSVERSIONINFOEXW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[128];
    WORD wServicePackMajor;
    WORD wServicePackMinor;
    WORD wSuiteMask;
    BYTE wProductType;
    BYTE wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;

#define VER_BUILDNUMBER 0x0000004
#define VER_MAJORVERSION 0x0000002
#define VER_MINORVERSION 0x0000001
#define VER_PLATFORMID 0x0000008
#define VER_SERVICEPACKMAJOR 0x0000020
#define VER_SERVICEPACKMINOR 0x0000010
#define VER_SUITENAME 0x0000040
#define VER_PRODUCT_TYPE 0x0000080

#define VER_EQUAL 1
#define VER_GREATER 2
#define VER_GREATER_EQUAL 3
#define VER_LESS 4
#define VER_LESS_EQUAL 5

#define _WIN32_WINNT_NT4 0x0400           // Windows NT 4.0
#define _WIN32_WINNT_WIN2K 0x0500         // Windows 2000
#define _WIN32_WINNT_WINXP 0x0501         // Windows XP
#define _WIN32_WINNT_WS03 0x0502          // Windows Server 2003
#define _WIN32_WINNT_WIN6 0x0600          // Windows Vista
#define _WIN32_WINNT_VISTA 0x0600         // Windows Vista
#define _WIN32_WINNT_WS08 0x0600          // Windows Server 2008
#define _WIN32_WINNT_LONGHORN 0x0600      // Windows Vista
#define _WIN32_WINNT_WIN7 0x0601          // Windows 7
#define _WIN32_WINNT_WIN8 0x0602          // Windows 8
#define _WIN32_WINNT_WINBLUE 0x0603       // Windows 8.1
#define _WIN32_WINNT_WINTHRESHOLD 0x0A00  // Windows 10
#define _WIN32_WINNT_WIN10 0x0A00         // Windows 10

#define NTSYSAPI __declspec(dllimport)

#define HIBYTE(w) ((BYTE) (((WORD) (w) >> 8) & 0xFF))
#define LOBYTE(w) ((BYTE) (w))

#define DRIVERVERSION 0
#define TECHNOLOGY 2
#define HORZSIZE 4
#define VERTSIZE 6
#define HORZRES 8
#define VERTRES 10
#define LOGPIXELSX 88
#define LOGPIXELSY 90
#define BITSPIXEL 12
#define PLANES 14
#define NUMBRUSHES 16
#define NUMPENS 18
#define NUMFONTS 22
#define NUMCOLORS 24
#define ASPECTX 40
#define ASPECTY 42
#define ASPECTXY 44
#define CLIPCAPS 36
#define SIZEPALETTE 104
#define NUMRESERVED 106
#define COLORRES 108
#define PHYSICALWIDTH 110
#define PHYSICALHEIGHT 111
#define PHYSICALOFFSETX 112
#define PHYSICALOFFSETY 113
#define SCALINGFACTORX 114
#define SCALINGFACTORY 115
#define RASTERCAPS 38
#define CURVECAPS 28
#define LINECAPS 30
#define POLYGONALCAPS 32
#define TEXTCAPS 34

typedef struct _POINTL {
    LONG x;
    LONG y;
} POINTL, *PPOINTL;

typedef struct _BORDERWIDTHS {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} BORDERWIDTHS, *LPBORDERWIDTHS;

using LPCBORDERWIDTHS = const BORDERWIDTHS *;

using RECT    = BORDERWIDTHS;
using LPRECT  = LPBORDERWIDTHS;
using LPCRECT = LPCBORDERWIDTHS;

#define CCHDEVICENAME 32
#define CCHFORMNAME 32

typedef struct _devicemodeW {
    WCHAR dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;

    union {
        struct {
            short dmOrientation;
            short dmPaperSize;
            short dmPaperLength;
            short dmPaperWidth;
            short dmScale;
            short dmCopies;
            short dmDefaultSource;
            short dmPrintQuality;
        } DUMMYSTRUCTNAME;

        POINTL dmPosition;

        struct {
            POINTL dmPosition;
            DWORD dmDisplayOrientation;
            DWORD dmDisplayFixedOutput;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME;

    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR dmFormName[CCHFORMNAME];
    WORD dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;

    union {
        DWORD dmDisplayFlags;
        DWORD dmNup;
    } DUMMYUNIONNAME2;

    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
    DWORD dmPanningWidth;
    DWORD dmPanningHeight;
} DEVMODEW, *PDEVMODEW, *NPDEVMODEW, *LPDEVMODEW;

typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

typedef struct tagMONITORINFOEXW : tagMONITORINFO {
    WCHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;

#define CALLBACK __stdcall
#define WINAPI __stdcall
#define IMAGEAPI __stdcall
#define NTAPI __stdcall

typedef BOOL(CALLBACK *MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

#define DM_BITSPERPEL 0x40000
#define DM_PELSWIDTH 0x80000
#define DM_PELSHEIGHT 0x100000
#define DM_DISPLAYFREQUENCY 0x400000

#define CDS_TEST 2
#define CDS_FULLSCREEN 4

#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_RESTART 1
#define DISP_CHANGE_BADFLAGS (-4)
#define DISP_CHANGE_BADPARAM (-5)
#define DISP_CHANGE_BADDUALVIEW (-6)
#define DISP_CHANGE_FAILED (-1)
#define DISP_CHANGE_BADMODE (-2)
#define DISP_CHANGE_NOTUPDATED (-3)

#define DISPLAY_DEVICE_ACTIVE 0x00000001
#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP 0x00000001
#define DISPLAY_DEVICE_MULTI_DRIVER 0x00000002
#define DISPLAY_DEVICE_PRIMARY_DEVICE 0x00000004
#define DISPLAY_DEVICE_MIRRORING_DRIVER 0x00000008
#define DISPLAY_DEVICE_VGA_COMPATIBLE 0x00000010
#define DISPLAY_DEVICE_MODESPRUNED 0x8000000

#define MONITOR_DEFAULTTONULL 0x00000000
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define MONITOR_DEFAULTTONEAREST 0x00000002
#define MONITORINFOF_PRIMARY 0x00000001

#define EDS_ROTATEDMODE 0x00000004

typedef struct _DISPLAY_DEVICEW {
    DWORD cb;
    WCHAR DeviceName[32];
    WCHAR DeviceString[128];
    DWORD StateFlags;
    WCHAR DeviceID[128];
    WCHAR DeviceKey[128];
} DISPLAY_DEVICEW, *PDISPLAY_DEVICEW, *LPDISPLAY_DEVICEW;

#define USER_DEFAULT_SCREEN_DPI 96

extern "C" {
HDC GetDC(
    HWND hWnd);

int ReleaseDC(
    HWND hWnd,
    HDC hDC);

BOOL InvalidateRect(
    HWND hWnd,
    const RECT *lpRect,
    BOOL bErase);

HMONITOR MonitorFromWindow(
    HWND hwnd,
    DWORD dwFlags);

BOOL EnumDisplayDevicesW(
    LPCWSTR lpDevice,
    DWORD iDevNum,
    PDISPLAY_DEVICEW lpDisplayDevice,
    DWORD dwFlags);

LONG ChangeDisplaySettingsExW(
    LPCWSTR lpszDeviceName,
    DEVMODEW *lpDevMode,
    HWND hwnd,
    DWORD dwflags,
    LPVOID lParam);

BOOL GetMonitorInfoW(
    HMONITOR hMonitor,
    LPMONITORINFO lpmi);

BOOL EnumDisplayMonitors(
    HDC hdc,
    LPCRECT lprcClip,
    MONITORENUMPROC lpfnEnum,
    LPARAM dwData);

BOOL EnumDisplaySettingsW(
    LPCWSTR lpszDeviceName,
    DWORD iModeNum,
    DEVMODEW *lpDevMode);

BOOL EnumDisplaySettingsExW(
    LPCWSTR lpszDeviceName,
    DWORD iModeNum,
    DEVMODEW *lpDevMode,
    DWORD dwFlags);

int GetDeviceCaps(
    HDC hdc,
    int index);

BOOL DeleteDC(
    HDC hdc);

HDC CreateDCW(
    LPCWSTR pwszDriver,
    LPCWSTR pwszDevice,
    LPCWSTR pszPort,
    const DEVMODEW *pdm);

NTSYSAPI ULONGLONG VerSetConditionMask(
    ULONGLONG ConditionMask,
    DWORD TypeMask,
    BYTE Condition);

BOOL SetProcessDpiAwarenessContext(
    DPI_AWARENESS_CONTEXT value);

BOOL SetProcessDPIAware();

BOOL SystemParametersInfoW(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni);
}

#define ENUM_CURRENT_SETTINGS 0xFFFFFFFF

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _LARGE_INTEGER {
    union {
        struct {
            ULONG LowPart;
            LONG HighPart;
        };

        INT64 QuadPart;
    };
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;

    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        } DUMMYSTRUCTNAME;

        PVOID Pointer;
    } DUMMYUNIONNAME;

    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _IMAGE_DOS_HEADER {
    WORD e_magic;
    WORD e_cblp;
    WORD e_cp;
    WORD e_crlc;
    WORD e_cparhdr;
    WORD e_minalloc;
    WORD e_maxalloc;
    WORD e_ss;
    WORD e_sp;
    WORD e_csum;
    WORD e_ip;
    WORD e_cs;
    WORD e_lfarlc;
    WORD e_ovno;
    WORD e_res[4];
    WORD e_oemid;
    WORD e_oeminfo;
    WORD e_res2[10];
    LONG e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _GUID {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;

typedef LRESULT(CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

using HBRUSH    = HANDLE;
using HICON     = HANDLE;
using HCURSOR   = HICON;
using HDC       = HANDLE;
using HINSTANCE = HANDLE;

typedef struct tagWNDCLASSEXW {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCWSTR lpszMenuName;
    LPCWSTR lpszClassName;
    HICON hIconSm;
} WNDCLASSEXW, *PWNDCLASSEXW, *NPWNDCLASSEXW, *LPWNDCLASSEXW;

using LPOLESTR = wchar_t *;

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;

using PCONSOLE_SCREEN_BUFFER_INFO = CONSOLE_SCREEN_BUFFER_INFO *;

#define ATTACH_PARENT_PROCESS ((DWORD) -1)

#define STD_INPUT_HANDLE ((DWORD) -10)
#define STD_OUTPUT_HANDLE ((DWORD) -11)
#define STD_ERROR_HANDLE ((DWORD) -12)

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004

#define SW_HIDE 0
#define SW_NORMAL 1
#define SW_SHOWNORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_MAXIMIZE 3
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11
#define SW_MAX 11

#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_DBLCLKS 0x0008
#define CS_OWNDC 0x0020
#define CS_CLASSDC 0x0040
#define CS_PARENTDC 0x0080
#define CS_NOCLOSE 0x0200
#define CS_SAVEBITS 0x0800
#define CS_BYTEALIGNCLIENT 0x1000
#define CS_BYTEALIGNWINDOW 0x2000
#define CS_GLOBALCLASS 0x4000

using TCHAR   = wchar;
using LPTSTR  = TCHAR *;
using LPCTSTR = const TCHAR *;

#define MAKEINTRESOURCE(i) (LPTSTR)((DWORD) ((WORD) (i)))

/*
 * Standard Cursor IDs
 */
#define IDC_ARROW MAKEINTRESOURCE(32512)
#define IDC_IBEAM MAKEINTRESOURCE(32513)
#define IDC_WAIT MAKEINTRESOURCE(32514)
#define IDC_CROSS MAKEINTRESOURCE(32515)
#define IDC_UPARROW MAKEINTRESOURCE(32516)
#define IDC_SIZE MAKEINTRESOURCE(32640) /* OBSOLETE: use IDC_SIZEALL */
#define IDC_ICON MAKEINTRESOURCE(32641) /* OBSOLETE: use IDC_ARROW */
#define IDC_SIZENWSE MAKEINTRESOURCE(32642)
#define IDC_SIZENESW MAKEINTRESOURCE(32643)
#define IDC_SIZEWE MAKEINTRESOURCE(32644)
#define IDC_SIZENS MAKEINTRESOURCE(32645)
#define IDC_SIZEALL MAKEINTRESOURCE(32646)
#define IDC_NO MAKEINTRESOURCE(32648) /*not in win3.1 */
#define IDC_HAND MAKEINTRESOURCE(32649)
#define IDC_APPSTARTING MAKEINTRESOURCE(32650) /*not in win3.1 */
#define IDC_HELP MAKEINTRESOURCE(32651)

#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define IDI_HAND MAKEINTRESOURCE(32513)
#define IDI_QUESTION MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION MAKEINTRESOURCE(32515)
#define IDI_ASTERISK MAKEINTRESOURCE(32516)
#define IDI_WINLOGO MAKEINTRESOURCE(32517)

#define IMAGE_BITMAP 0
#define IMAGE_ICON 1
#define IMAGE_CURSOR 2
#define IMAGE_ENHMETAFILE 3

#define LR_DEFAULTCOLOR 0
#define LR_MONOCHROME 1
#define LR_COLOR 2
#define LR_COPYRETURNORG 4
#define LR_COPYDELETEORG 8
#define LR_LOADFROMFILE 16
#define LR_LOADTRANSPARENT 32
#define LR_LOADREALSIZE 128
#define LR_LOADMAP3DCOLORS 4096
#define LR_CREATEDIBSECTION 8192
#define LR_COPYFROMRESOURCE 0x4000
#define LR_SHARED 32768
#define LR_DEFAULTSIZE 64

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    DWORD lPrivate;
} MSG, *PMSG, *NPMSG, *LPMSG;

extern "C" {
DWORD GetCurrentThreadId();

BOOL DestroyWindow(HWND hWnd);
HMODULE LoadLibraryW(LPCWSTR lpLibFileName);
BOOL FreeLibrary(HMODULE hLibModule);
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);

HRESULT CoCreateGuid(GUID *pguid);
HRESULT StringFromCLSID(GUID rclsid, LPOLESTR *lplpsz);

LRESULT DefWindowProcW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

HCURSOR LoadCursorW(
    HINSTANCE hInstance,
    LPCWSTR lpCursorName);

HANDLE LoadImageW(
    HINSTANCE hInst,
    LPCWSTR name,
    UINT type,
    int cx,
    int cy,
    UINT fuLoad);

ATOM RegisterClassExW(
    const WNDCLASSEXW *Arg1);

BOOL WINAPI AttachConsole(
    DWORD dwProcessId);

HANDLE WINAPI GetStdHandle(
    DWORD nStdHandle);

BOOL WINAPI AllocConsole(void);

BOOL WINAPI GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo);

BOOL WINAPI SetConsoleScreenBufferSize(
    HANDLE hConsoleOutput,
    COORD dwSize);

BOOL WINAPI SetConsoleOutputCP(
    UINT wCodePageID);

BOOL WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped);

BOOL WINAPI SetConsoleMode(
    HANDLE hConsoleHandle,
    DWORD dwMode);

BOOL WINAPI GetConsoleMode(
    HANDLE hConsoleHandle,
    LPDWORD lpMode);

DWORD GetLastError();

DWORD GetModuleFileNameW(
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize);

BOOL QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency);

LPWSTR *CommandLineToArgvW(
    LPCWSTR lpCmdLine,
    int *pNumArgs);

LPWSTR GetCommandLineW();

HLOCAL LocalFree(
    HLOCAL hMem);

HWND CreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

BOOL ShowWindow(
    HWND hWnd,
    int nCmdShow);

BOOL WINAPI PeekMessageW(LPMSG lpMsg,
                         HWND hwnd,
                         UINT uMsgFilterMin,
                         UINT uMsgFilterMax,
                         UINT wRemoveMsg);
BOOL WINAPI TranslateMessage(CONST MSG *lpMsg);
LONG WINAPI DispatchMessageW(CONST MSG *lpMsg);

LRESULT WINAPI CallWindowProc(WNDPROC lpPrevWndFunc,
                              HWND hwnd,
                              UINT Msg,
                              WPARAM wParam,
                              LPARAM lParam);
BOOL WINAPI PostMessage(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI PostThreadMessage(DWORD dwThreadId,
                              UINT Msg,
                              WPARAM wParam,
                              LPARAM lParam);
VOID WINAPI PostQuitMessage(int nExitCode);

BOOL ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped);
}

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_W {
    DWORD dbcc_size;
    DWORD dbcc_devicetype;
    DWORD dbcc_reserved;
    GUID dbcc_classguid;
    wchar_t dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_W, *PDEV_BROADCAST_DEVICEINTERFACE_W;

typedef struct _DEV_BROADCAST_HDR {
    DWORD dbch_size;
    DWORD dbch_devicetype;
    DWORD dbch_reserved;
} DEV_BROADCAST_HDR;

#define DBT_DEVTYP_DEVICEINTERFACE 0x00000005
#define DBT_DEVTYP_HANDLE 0x00000006
#define DBT_DEVTYP_OEM 0x00000000
#define DBT_DEVTYP_PORT 0x00000003
#define DBT_DEVTYP_VOLUME 0x00000002

#define DEVICE_NOTIFY_WINDOW_HANDLE 0
#define DEVICE_NOTIFY_SERVICE_HANDLE 1
#define DEVICE_NOTIFY_ALL_INTERFACE_CLASSES 4

/* PeekMessage options*/
#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define PM_NOYIELD 0x0002

#define SIZE_RESTORED 0
#define SIZE_MINIMIZED 1
#define SIZE_MAXIMIZED 2
#define SIZE_MAXSHOW 3
#define SIZE_MAXHIDE 4

/*
 * System Menu Command Values
 */
#define SC_SIZE 0xF000
#define SC_MOVE 0xF010
#define SC_MINIMIZE 0xF020
#define SC_MAXIMIZE 0xF030
#define SC_NEXTWINDOW 0xF040
#define SC_PREVWINDOW 0xF050
#define SC_CLOSE 0xF060
#define SC_VSCROLL 0xF070
#define SC_HSCROLL 0xF080
#define SC_MOUSEMENU 0xF090
#define SC_KEYMENU 0xF100
#define SC_ARRANGE 0xF110
#define SC_RESTORE 0xF120
#define SC_TASKLIST 0xF130
#define SC_SCREENSAVE 0xF140
#define SC_HOTKEY 0xF150

/* WM_ACTIVATE state values*/
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2

/* WM_NCHITTEST codes*/
#define HTERROR (-2)
#define HTTRANSPARENT (-1)
#define HTNOWHERE 0
#define HTCLIENT 1
#define HTCAPTION 2
#define HTSYSMENU 3
#define HTGROWBOX 4
#define HTSIZE HTGROWBOX
#define HTMENU 5
#define HTHSCROLL 6
#define HTVSCROLL 7
#define HTMINBUTTON 8
#define HTMAXBUTTON 9
#define HTLEFT 10
#define HTRIGHT 11
#define HTTOP 12
#define HTTOPLEFT 13
#define HTTOPRIGHT 14
#define HTBOTTOM 15
#define HTBOTTOMLEFT 16
#define HTBOTTOMRIGHT 17
#define HTBORDER 18
#define HTREDUCE HTMINBUTTON
#define HTZOOM HTMAXBUTTON
#define HTSIZEFIRST HTLEFT
#define HTSIZELAST HTBOTTOMRIGHT
#define HTOBJECT 19
#define HTCLOSE 20
#define HTHELP 21

#define LOWORD(l) ((WORD) (((DWORD_PTR) (l)) & 0xffff))
#define HIWORD(l) ((WORD) ((((DWORD_PTR) (l)) >> 16) & 0xffff))

#define HIDWORD(dw, hw) LOWORD(dw) | (hw << 16)
#define LODWORD(dw, lw) (HIWORD(dw) << 16) | lw

#define POINTSTOPOINT(pt, pts)                          \
    {                                                   \
        (pt).x = (LONG) (SHORT) LOWORD(*(LONG *) &pts); \
        (pt).y = (LONG) (SHORT) HIWORD(*(LONG *) &pts); \
    }

#define POINTTOPOINTS(pt) (MAKELONG((short) ((pt).x), (short) ((pt).y)))
#define MAKEWPARAM(l, h) (WPARAM) MAKELONG(l, h)
#define MAKELPARAM(l, h) (LPARAM) MAKELONG(l, h)
#define MAKELRESULT(l, h) (LRESULT) MAKELONG(l, h)

/* window messages*/
#define SC_MONITORPOWER 0xF170

#define WM_NULL 0x0000
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_NCDESTROY WM_DESTROY
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_ENABLE 0x000A
#define WM_SETREDRAW 0x000B
#define WM_SETTEXT 0x000C
#define WM_GETTEXT 0x000D
#define WM_GETTEXTLENGTH 0x000E
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_ERASEBKGND 0x0014
#define WM_SHOWWINDOW 0x0018
#define WM_CTLCOLOR 0x0019
#define WM_NEXTDLGCTL 0x0028
#define WM_DRAWITEM 0x002B
#define WM_MEASUREITEM 0x002C
#define WM_DELETEITEM 0x002D
#define WM_VKEYTOITEM 0x002E
#define WM_CHARTOITEM 0x002F
#define WM_SETFONT 0x0030
#define WM_GETFONT 0x0031
#define WM_COMPAREITEM 0x0039
#define WM_WINDOWPOSCHANGED 0x0047
#define WM_NOTIFY 0x004E
#define WM_NCCALCSIZE 0x0083
#define WM_NCHITTEST 0x0084
#define WM_NCPAINT 0x0085
#define WM_GETDLGCODE 0x0087
#define WM_NCMOUSEMOVE 0x00A0
#define WM_NCLBUTTONDOWN 0x00A1
#define WM_NCLBUTTONUP 0x00A2
#define WM_NCLBUTTONDBLCLK 0x00A3
#define WM_NCRBUTTONDOWN 0x00A4
#define WM_NCRBUTTONUP 0x00A5
#define WM_NCRBUTTONDBLCLK 0x00A6
#define WM_KEYFIRST 0x0100
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_DEADCHAR 0x0103   /* notimp*/
#define WM_SYSKEYDOWN 0x0104 /* nyi*/
#define WM_SYSKEYUP 0x0105   /* nyi*/
#define WM_SYSCHAR 0x0106
#define WM_SYSDEADCHAR 0x0107 /* notimp*/
#define WM_KEYLAST 0x0108
#define WM_INITDIALOG 0x0110
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define WM_TIMER 0x0113
#define WM_HSCROLL 0x0114
#define WM_VSCROLL 0x0115

#define WM_ENTERIDLE 0x0121

#define WM_CTLCOLORMSGBOX 0x0132
#define WM_CTLCOLOREDIT 0x0133
#define WM_CTLCOLORLISTBOX 0x0134
#define WM_CTLCOLORBTN 0x0135
#define WM_CTLCOLORDLG 0x0136
#define WM_CTLCOLORSCROLLBAR 0x0137
#define WM_CTLCOLORSTATIC 0x0138

#define WM_MOUSEFIRST 0x0200
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_MOUSELAST 0x020A
#define WM_HOTKEY 0x0312

#define WM_CARET_CREATE 0x03E0  /* Microwindows only*/
#define WM_CARET_DESTROY 0x03E1 /* Microwindows only*/
#define WM_CARET_BLINK 0x03E2   /* Microwindows only*/
#define WM_FDINPUT 0x03F0       /* Microwindows only*/
#define WM_FDOUTPUT 0x03F1      /* Microwindows only*/
#define WM_FDEXCEPT 0x03F2      /* Microwindows only*/
#define WM_USER 0x0400

/* Button codes for MW_MOUSEMOVED:
 * Please note that they differ from normal Windows codes
 */
#define MK_LBUTTON MWBUTTON_L
#define MK_RBUTTON MWBUTTON_R
#define MK_MBUTTON MWBUTTON_M

/*
 * Window Styles
 */
#define WS_OVERLAPPED 0x00000000L
#define WS_POPUP 0x80000000L
#define WS_CHILD 0x40000000L
#define WS_MINIMIZE 0x20000000L
#define WS_VISIBLE 0x10000000L
#define WS_DISABLED 0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_MAXIMIZE 0x01000000L
#define WS_CAPTION 0x00C00000L /* WS_BORDER | WS_DLGFRAME  */
#define WS_BORDER 0x00800000L
#define WS_DLGFRAME 0x00400000L
#define WS_VSCROLL 0x00200000L
#define WS_HSCROLL 0x00100000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_GROUP 0x00020000L
#define WS_TABSTOP 0x00010000L

#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L

#define WS_TILED WS_OVERLAPPED
#define WS_ICONIC WS_MINIMIZE
#define WS_SIZEBOX WS_THICKFRAME
#define WS_TILEDWINDOW WS_OVERLAPPEDWINDOW

/*
 * Common Window Styles
 */
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED |  \
                             WS_CAPTION |     \
                             WS_SYSMENU |     \
                             WS_THICKFRAME |  \
                             WS_MINIMIZEBOX | \
                             WS_MAXIMIZEBOX)

#define WS_POPUPWINDOW (WS_POPUP |  \
                        WS_BORDER | \
                        WS_SYSMENU)

#define WS_CHILDWINDOW (WS_CHILD)

/*
 * Extended Window Styles
 */
#define WS_EX_DLGMODALFRAME 0x00000001L
#define WS_EX_NOPARENTNOTIFY 0x00000004L
#define WS_EX_TOPMOST 0x00000008L
#define WS_EX_ACCEPTFILES 0x00000010L
#define WS_EX_TRANSPARENT 0x00000020L
#define WS_EX_MDICHILD 0x00000040L
#define WS_EX_TOOLWINDOW 0x00000080L
#define WS_EX_WINDOWEDGE 0x00000100L
#define WS_EX_CLIENTEDGE 0x00000200L
#define WS_EX_CONTEXTHELP 0x00000400L

#define WS_EX_RIGHT 0x00001000L
#define WS_EX_LEFT 0x00000000L
#define WS_EX_RTLREADING 0x00002000L
#define WS_EX_LTRREADING 0x00000000L
#define WS_EX_LEFTSCROLLBAR 0x00004000L
#define WS_EX_RIGHTSCROLLBAR 0x00000000L

#define WS_EX_CONTROLPARENT 0x00010000L
#define WS_EX_STATICEDGE 0x00020000L
#define WS_EX_APPWINDOW 0x00040000L
#define WS_EX_LAYERED 0x00080000L

#define WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

#define CW_USEDEFAULT ((int) 0x80000000)

#define HWND_DESKTOP ((HWND) 0)

typedef enum _HEAP_INFORMATION_CLASS {
    HeapCompatibilityInformation,
    HeapEnableTerminationOnCorruption,
    HeapOptimizeResources
} HEAP_INFORMATION_CLASS;

extern "C" {
HDEVNOTIFY RegisterDeviceNotificationW(
    HANDLE hRecipient,
    LPVOID NotificationFilter,
    DWORD Flags);

HANDLE GetProcessHeap();

LPVOID HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    SIZE_T dwBytes);

BOOL HeapQueryInformation(
    HANDLE HeapHandle,
    HEAP_INFORMATION_CLASS HeapInformationClass,
    PVOID HeapInformation,
    SIZE_T HeapInformationLength,
    PSIZE_T ReturnLength);

LPVOID HeapReAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem,
    SIZE_T dwBytes);
}

#define HEAP_GENERATE_EXCEPTIONS 0x00000004
#define HEAP_NO_SERIALIZE 0x00000001
#define HEAP_REALLOC_IN_PLACE_ONLY 0x00000010
#define HEAP_ZERO_MEMORY 0x00000008

#define STATUS_NONCONTINUABLE_EXCEPTION 0xC0000025
#define STATUS_INVALID_DISPOSITION 0xC0000026
#define STATUS_UNWIND 0xC0000027
#define STATUS_BAD_STACK 0xC0000028
#define STATUS_INVALID_UNWIND_TARGET 0xC0000029

#define STATUS_SUCCESS 0x00000000
#define STATUS_SOME_NOT_MAPPED 0x00000107
#define STATUS_NO_MEMORY 0xC0000017
#define STATUS_OBJECT_NAME_NOT_FOUND 0xC0000034
#define STATUS_NONE_MAPPED 0xC0000073
#define STATUS_INSUFFICIENT_RESOURCES 0xC000009A
#define STATUS_ACCESS_DENIED 0xC0000022

#define STATUS_GUARD_PAGE_VIOLATION 0x80000001

#define EH_NONCONTINUABLE 0x01
#define EH_UNWINDING 0x02
#define EH_EXIT_UNWIND 0x04
#define EH_STACK_INVALID 0x08
#define EH_NESTED_CALL 0x10

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

extern "C" {
SIZE_T HeapSize(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem);

HANDLE CreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName);

BOOL CloseHandle(
    HANDLE hObject);

LPVOID MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    SIZE_T dwNumberOfBytesToMap);

BOOL UnmapViewOfFile(
    LPCVOID lpBaseAddress);

HANDLE OpenFileMappingW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName);

BOOL HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem);

void ExitProcess(
    UINT uExitCode);

BOOL SetEnvironmentVariableW(
    LPCWSTR lpName,
    LPCWSTR lpValue);

DWORD GetEnvironmentVariableW(
    LPCWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize);

BOOL SetCurrentDirectoryW(
    LPCTSTR lpPathName);

DWORD GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPTSTR lpBuffer);

BOOL QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount);

BOOL OpenClipboard(
    HWND hWndNewOwner);

BOOL CloseClipboard();

HANDLE GetClipboardData(
    UINT uFormat);

LPVOID GlobalLock(
    HGLOBAL hMem);

BOOL GlobalUnlock(
    HGLOBAL hMem);

HGLOBAL GlobalAlloc(
    UINT uFlags,
    SIZE_T dwBytes);

HGLOBAL GlobalFree(
    HGLOBAL hMem);

HANDLE SetClipboardData(
    UINT uFormat,
    HANDLE hMem);

BOOL EmptyClipboard();

DWORD GetCurrentProcessId();
}

#define FILE_MAP_WRITE 0x0002
#define FILE_MAP_READ 0x0004

#define PAGE_READWRITE 0x04

#define CF_UNICODETEXT 13

#define GHND 0x0042
#define GMEM_FIXED 0x0000
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
#define GPTR 0x0040

#define EXCEPTION_NONCONTINUABLE 1
#define EXCEPTION_MAXIMUM_PARAMETERS 15
#define MAXIMUM_SUPPORTED_EXTENSION 512
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_EXECUTE_HANDLER 1

/*
 * Exception codes
 */

#define STATUS_WAIT_0 0x00000000
#define STATUS_ABANDONED_WAIT_0 0x00000080
#define STATUS_USER_APC 0x000000C0
#define STATUS_TIMEOUT 0x00000102
#define STATUS_PENDING 0x00000103
#define STATUS_DATATYPE_MISALIGNMENT 0x80000002
#define STATUS_BREAKPOINT 0x80000003
#define STATUS_SINGLE_STEP 0x80000004
#define STATUS_ACCESS_VIOLATION 0xC0000005
#define STATUS_IN_PAGE_ERROR 0xC0000006
#define STATUS_NO_MEMORY 0xC0000017
#define STATUS_ILLEGAL_INSTRUCTION 0xC000001D
#define STATUS_NONCONTINUABLE_EXCEPTION 0xC0000025
#define STATUS_INVALID_DISPOSITION 0xC0000026
#define STATUS_ARRAY_BOUNDS_EXCEEDED 0xC000008C
#define STATUS_FLOAT_DENORMAL_OPERAND 0xC000008D
#define STATUS_FLOAT_DIVIDE_BY_ZERO 0xC000008E
#define STATUS_FLOAT_INEXACT_RESULT 0xC000008F
#define STATUS_FLOAT_INVALID_OPERATION 0xC0000090
#define STATUS_FLOAT_OVERFLOW 0xC0000091
#define STATUS_FLOAT_STACK_CHECK 0xC0000092
#define STATUS_FLOAT_UNDERFLOW 0xC0000093
#define STATUS_INTEGER_DIVIDE_BY_ZERO 0xC0000094
#define STATUS_INTEGER_OVERFLOW 0xC0000095
#define STATUS_PRIVILEGED_INSTRUCTION 0xC0000096
#define STATUS_STACK_OVERFLOW 0xC00000FD
#define STATUS_CONTROL_C_EXIT 0xC000013A
#define STATUS_INVALID_HANDLE 0xC0000008
#define STATUS_POSSIBLE_DEADLOCK 0xC0000194

#define EXCEPTION_ACCESS_VIOLATION STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR STATUS_IN_PAGE_ERROR

#define EXCEPTION_ILLEGAL_INSTRUCTION STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE STATUS_INVALID_HANDLE
#define EXCEPTION_POSSIBLE_DEADLOCK STATUS_POSSIBLE_DEADLOCK

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];

} EXCEPTION_RECORD, *PEXCEPTION_RECORD, *LPEXCEPTION_RECORD;

typedef struct _M128A {
    unsigned __int64 Low;
    __int64 High;
} M128A, *PM128A;

typedef struct XMM_SAVE_AREA32 {
    UINT16 ControlWord;
    UINT16 StatusWord;
    UINT8 TagWord;
    UINT8 Reserved1;
    UINT16 ErrorOpcode;
    ULONG32 ErrorOffset;
    UINT16 ErrorSelector;
    UINT16 Reserved2;
    ULONG32 DataOffset;
    UINT16 DataSelector;
    UINT16 Reserved3;
    ULONG32 MxCsr;
    ULONG32 MxCsr_Mask;
    struct _M128A FloatRegisters[8];
    struct _M128A XmmRegisters[16];
    UINT8 Reserved4[96];
} XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

typedef struct _NEON128 {
    ULONGLONG Low;
    LONGLONG High;
} NEON128, *PNEON128;

typedef struct _CONTEXT {
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;
    DWORD ContextFlags;
    DWORD MxCsr;
    WORD SegCs;
    WORD SegDs;
    WORD SegEs;
    WORD SegFs;
    WORD SegGs;
    WORD SegSs;
    DWORD EFlags;
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;
    DWORD64 Rip;

    union {
        XMM_SAVE_AREA32 FltSave;
        NEON128 Q[16];
        ULONGLONG D[32];

        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        } DUMMYSTRUCTNAME;

        DWORD S[32];
    } DUMMYUNIONNAME;

    M128A VectorRegister[26];
    DWORD64 VectorControl;
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;

} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *LPEXCEPTION_POINTERS;

typedef enum {
    AddrMode1616,
    AddrMode1632,
    AddrModeReal,
    AddrModeFlat
} ADDRESS_MODE;

typedef struct _KDHELP64 {
    DWORD64 Thread;
    DWORD ThCallbackStack;
    DWORD ThCallbackBStore;
    DWORD NextCallback;
    DWORD FramePointer;
    DWORD64 KiCallUserMode;
    DWORD64 KeUserCallbackDispatcher;
    DWORD64 SystemRangeStart;
    DWORD64 KiUserExceptionDispatcher;
    DWORD64 StackBase;
    DWORD64 StackLimit;
    DWORD BuildVersion;
    DWORD RetpolineStubFunctionTableSize;
    DWORD64 RetpolineStubFunctionTable;
    DWORD RetpolineStubOffset;
    DWORD RetpolineStubSize;
    DWORD64 Reserved0[2];
} KDHELP64, *PKDHELP64;

typedef struct _tagADDRESS64 {
    DWORD64 Offset;
    WORD Segment;
    ADDRESS_MODE Mode;
} ADDRESS64, *LPADDRESS64;

typedef struct _tagSTACKFRAME64 {
    ADDRESS64 AddrPC;
    ADDRESS64 AddrReturn;
    ADDRESS64 AddrFrame;
    ADDRESS64 AddrStack;
    ADDRESS64 AddrBStore;
    PVOID FuncTableEntry;
    DWORD64 Params[4];
    BOOL Far;
    BOOL Virtual;
    DWORD64 Reserved[3];
    KDHELP64 KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;

typedef struct _SYMBOL_INFO {
    ULONG SizeOfStruct;
    ULONG TypeIndex;
    ULONG64 Reserved[2];
    ULONG Index;
    ULONG Size;
    ULONG64 ModBase;
    ULONG Flags;
    ULONG64 Value;
    ULONG64 Address;
    ULONG Register;
    ULONG Scope;
    ULONG Tag;
    ULONG NameLen;
    ULONG MaxNameLen;
    CHAR Name[1];
} SYMBOL_INFO, *PSYMBOL_INFO;

typedef BOOL(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE64)(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);

typedef PVOID(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE64)(HANDLE ahProcess, DWORD64 AddrBase);

typedef DWORD64(__stdcall *PGET_MODULE_BASE_ROUTINE64)(HANDLE hProcess, DWORD64 Address);

typedef DWORD64(__stdcall *PTRANSLATE_ADDRESS_ROUTINE64)(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr);

typedef struct _IMAGEHLP_LINEW64 {
    DWORD SizeOfStruct;
    PVOID Key;
    DWORD LineNumber;
    PWSTR FileName;
    DWORD64 Address;
} IMAGEHLP_LINEW64, *PIMAGEHLP_LINEW64;

typedef LONG(CALLBACK *PTOP_LEVEL_EXCEPTION_FILTER)(LPEXCEPTION_POINTERS);
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

extern "C" {
HANDLE GetCurrentProcess();

BOOL IMAGEAPI SymInitialize(
    HANDLE hProcess,
    PCSTR UserSearchPath,
    BOOL fInvadeProcess);

BOOL IMAGEAPI SymCleanup(
    HANDLE hProcess);

HANDLE GetCurrentThread();

PVOID IMAGEAPI SymFunctionTableAccess64(
    HANDLE hProcess,
    DWORD64 AddrBase);

DWORD64 IMAGEAPI SymGetModuleBase64(
    HANDLE hProcess,
    DWORD64 qwAddr);

BOOL IMAGEAPI StackWalk64(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME64 StackFrame,
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);

BOOL IMAGEAPI SymFromAddr(
    HANDLE hProcess,
    DWORD64 Address,
    PDWORD64 Displacement,
    PSYMBOL_INFO Symbol);

BOOL IMAGEAPI SymGetLineFromAddrW64(
    HANDLE hProcess,
    DWORD64 dwAddr,
    PDWORD pdwDisplacement,
    PIMAGEHLP_LINEW64 Line);

LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
}

#define MAX_SYM_NAME 2000

#define IMAGE_FILE_MACHINE_I386 0x014c
#define IMAGE_FILE_MACHINE_IA64 0x0200
#define IMAGE_FILE_MACHINE_AMD64 0x8664

#define CO_S_NOTALLINTERFACES 0x00080012

#define NTE_SILENT_CONTEXT 0x80090022

typedef struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[MAX_PATH];
    WCHAR cAlternateFileName[14];
    DWORD dwFileType;
    DWORD dwCreatorType;
    WORD wFinderFlags;
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

#define INVALID_FILE_ATTRIBUTES -1

#define SYMBOLIC_LINK_FLAG_DIRECTORY 1

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define GENERIC_EXECUTE 0x20000000
#define GENERIC_ALL 0x10000000

#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004

#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_ENCRYPTED 0x00004000
#define FILE_FLAG_OVERLAPPED 0x40000000

#define FILE_FLAG_BACKUP_SEMANTICS 0x02000000

#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

#define MOVEFILE_REPLACE_EXISTING 0x00000001
#define MOVEFILE_COPY_ALLOWED 0x00000002
#define MOVEFILE_DELAY_UNTIL_REBOOT 0x00000004
#define MOVEFILE_WRITE_THROUGH 0x00000008

#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_FLAG_FIRST_PIPE_INSTANCE 0x00080000

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef char *va_list;
#endif

#ifndef va_start
#if COMPILER == MSVC
#define va_start __crt_va_start
#define va_arg __crt_va_arg
#define va_end __crt_va_end
#define va_copy(destination, source) ((destination) = (source))
#else
#error Define for other compilers
#endif
#endif

extern "C" {
BOOL GetFileSizeEx(
    HANDLE hFile,
    PLARGE_INTEGER lpFileSize);

BOOL SetFilePointerEx(
    HANDLE hFile,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER lpNewFilePointer,
    DWORD dwMoveMethod);

BOOL SetEndOfFile(
    HANDLE hFile);

HANDLE FindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData);

BOOL GetFileInformationByHandle(
    HANDLE hFile,
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation);

DWORD GetFileAttributesW(
    LPCWSTR lpFileName);

BOOL GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime);

BOOL CreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL DeleteFileW(
    LPCWSTR lpFileName);

BOOL RemoveDirectoryW(
    LPCWSTR lpPathName);

BOOL CopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists);

BOOL CreateHardLinkW(
    LPCWSTR lpFileName,
    LPCWSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOLEAN CreateSymbolicLinkW(
    LPCWSTR lpSymlinkFileName,
    LPCWSTR lpTargetFileName,
    DWORD dwFlags);

BOOL FindNextFileW(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData);

BOOL MoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName);

BOOL MoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags);

BOOL FindClose(
    HANDLE hFindFile);

int WideCharToMultiByte(
    UINT CodePage,
    DWORD dwFlags,
    LPCWCH lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCCH lpDefaultChar,
    LPBOOL lpUsedDefaultChar);

int MultiByteToWideChar(
    UINT CodePage,
    DWORD dwFlags,
    LPCCH lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar);

HANDLE CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

DWORD FormatMessageW(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPWSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments);
}

#define MAKELANGID(p, s) (((DWORD) ((WORD) (s)) << 10) | (WORD) (p))

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000
#define FORMAT_MESSAGE_FROM_HMODULE 0x00000800
#define FORMAT_MESSAGE_FROM_STRING 0x00000400
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200

/*
 * Special identifiers
*/

#define LANG_NEUTRAL 0x00         // Default custom (MUI) locale language
#define LANG_USER_DEFAULT 0x01    // User default locale language
#define LANG_SYSTEM_DEFAULT 0x02  // System default locale language
#define LANG_INVARIANT 0x7F       // Invariant locale language

#define SUBLANG_NEUTRAL 0x00             // Neutral sublanguage
#define SUBLANG_INVARIANT 0x00           // Invariant sublanguage
#define SUBLANG_DEFAULT 0x01             // User default sublanguage
#define SUBLANG_SYS_DEFAULT 0x02         // System default sublanguage
#define SUBLANG_CUSTOM_DEFAULT 0x03      // Default custom sublanguage
#define SUBLANG_CUSTOM_UNSPECIFIED 0x04  // Unspecified custom sublanguage
#define SUBLANG_UI_CUSTOM_DEFAULT 0x05   // Default custom MUI sublanguage

typedef VOID(NTAPI *PIMAGE_TLS_CALLBACK)(
    PVOID DllHandle,
    DWORD Reason,
    PVOID Reserved);

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution = 0,
    ExceptionContinueSearch    = 1,
    ExceptionNestedException   = 2,
    ExceptionCollidedUnwind    = 3
} EXCEPTION_DISPOSITION;

#define IMAGE_DOS_SIGNATURE 0x5A4D     // MZ
#define IMAGE_OS2_SIGNATURE 0x454E     // NE
#define IMAGE_OS2_SIGNATURE_LE 0x454C  // LE
#define IMAGE_NT_SIGNATURE 0x00004550  // PE00

typedef struct _IMAGE_FILE_HEADER {
    WORD Machine;
    WORD NumberOfSections;
    DWORD TimeDateStamp;
    DWORD PointerToSymbolTable;
    DWORD NumberOfSymbols;
    WORD SizeOfOptionalHeader;
    WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD VirtualAddress;
    DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    WORD Magic;
    BYTE MajorLinkerVersion;
    BYTE MinorLinkerVersion;
    DWORD SizeOfCode;
    DWORD SizeOfInitializedData;
    DWORD SizeOfUninitializedData;
    DWORD AddressOfEntryPoint;
    DWORD BaseOfCode;
    ULONGLONG ImageBase;
    DWORD SectionAlignment;
    DWORD FileAlignment;
    WORD MajorOperatingSystemVersion;
    WORD MinorOperatingSystemVersion;
    WORD MajorImageVersion;
    WORD MinorImageVersion;
    WORD MajorSubsystemVersion;
    WORD MinorSubsystemVersion;
    DWORD Win32VersionValue;
    DWORD SizeOfImage;
    DWORD SizeOfHeaders;
    DWORD CheckSum;
    WORD Subsystem;
    WORD DllCharacteristics;
    ULONGLONG SizeOfStackReserve;
    ULONGLONG SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve;
    ULONGLONG SizeOfHeapCommit;
    DWORD LoaderFlags;
    DWORD NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b
#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE Name[IMAGE_SIZEOF_SHORT_NAME];

    union {
        DWORD PhysicalAddress;
        DWORD VirtualSize;
    } Misc;

    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD NumberOfRelocations;
    WORD NumberOfLinenumbers;
    DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_FIRST_SECTION(ntheader) ((PIMAGE_SECTION_HEADER) ((ULONG_PTR) ntheader + offset_of(IMAGE_NT_HEADERS64, OptionalHeader) + ((PIMAGE_NT_HEADERS64) (ntheader))->FileHeader.SizeOfOptionalHeader))
#define IMAGE_SCN_MEM_WRITE 0x80000000

struct _RTL_CRITICAL_SECTION;

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, PRLIST_ENTRY;

typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD Type;
    WORD CreatorBackTraceIndex;
    _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Flags;
    WORD CreatorBackTraceIndexHigh;
    WORD SpareUSHORT;
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;  // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;  // force size on 64-bit systems when packed
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
using LPCRITICAL_SECTION = CRITICAL_SECTION *;
using PCRITICAL_SECTION  = CRITICAL_SECTION *;

typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;

        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

typedef DWORD(__stdcall *LPTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter);

extern "C" {
void InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection);

void DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection);

void EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection);

void LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection);

BOOL TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection);

HANDLE CreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName);

DWORD WaitForMultipleObjects(
    DWORD nCount,
    const HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds);

BOOL SetEvent(
    HANDLE hEvent);

BOOL ResetEvent(
    HANDLE hEvent);

void ExitThread(
    DWORD dwExitCode);

BOOL GetModuleHandleExW(
    DWORD dwFlags,
    LPCWSTR lpModuleName,
    HMODULE *phModule);

HANDLE CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId);

DWORD WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds);

BOOL TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode);

void GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo);

void Sleep(
    DWORD dwMilliseconds);
}

#define WAIT_OBJECT_0 0
#define INFINITE 0xFFFFFFFF

#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 0x00000004

// SEH intrinsics
#define GetExceptionCode _exception_code
#define exception_code _exception_code
#define GetExceptionInformation() ((struct _EXCEPTION_POINTERS *) _exception_info())
#define exception_info() ((struct _EXCEPTION_POINTERS *) _exception_info())
#define AbnormalTermination _abnormal_termination
#define abnormal_termination _abnormal_termination

extern "C" {

unsigned long __cdecl _exception_code(void);
void *__cdecl _exception_info(void);
int __cdecl _abnormal_termination(void);
}

// Defined values for the exception filter expression
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION (-1)

typedef struct _RTL_CONDITION_VARIABLE {
    PVOID Ptr;
} RTL_CONDITION_VARIABLE, *PRTL_CONDITION_VARIABLE;

typedef RTL_CONDITION_VARIABLE CONDITION_VARIABLE, *PCONDITION_VARIABLE;

extern "C" {
void WakeAllConditionVariable(
    PCONDITION_VARIABLE ConditionVariable);

BOOL SleepConditionVariableCS(
    PCONDITION_VARIABLE ConditionVariable,
    PCRITICAL_SECTION CriticalSection,
    DWORD dwMilliseconds);

BOOL InitializeCriticalSectionAndSpinCount(
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount);

HMODULE GetModuleHandleW(
    LPCWSTR lpModuleName);

DWORD WaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable);
}

#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#define UIntToPtr(ui) ((VOID *) (UINT_PTR) ((unsigned int) ui))

#define SPIF_SENDCHANGE 2

#define WM_COPYGLOBALDATA 0x0049
#define WM_UNICHAR 0x0109
#define UNICODE_NOCHAR 0xFFFF
#define WM_DPICHANGED 0x02E0
#define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#define EDS_ROTATEDMODE 0x00000004
#define DISPLAY_DEVICE_ACTIVE 0x00000001
#define WM_GETDPISCALEDSIZE 0x02e4
#define USER_DEFAULT_SCREEN_DPI 96

#ifndef DWMAPI
#define DWMAPI extern "C" __declspec(dllimport) HRESULT WINAPI
#define DWMAPI_(type) extern "C" __declspec(dllimport) type WINAPI
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(hr) ((HRESULT) (hr) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr) ((HRESULT) (hr) < 0)
#endif

typedef struct _DWM_BLURBEHIND {
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

DWMAPI DwmEnableBlurBehindWindow(
    HWND hWnd,
    const DWM_BLURBEHIND *pBlurBehind);

DWMAPI DwmIsCompositionEnabled(
    BOOL *pfEnabled);

typedef struct tagCHANGEFILTERSTRUCT {
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;

extern "C" {
HRGN CreateRectRgn(
    int x1,
    int y1,
    int x2,
    int y2);

LONG GetWindowLongW(
    HWND hWnd,
    int nIndex);

LONG SetWindowLongW(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);

LONG_PTR SetWindowLongPtrW(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong);

BOOL SetLayeredWindowAttributes(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags);

BOOL DeleteObject(
    HGDIOBJ ho);

BOOL RedrawWindow(
    HWND hWnd,
    const RECT *lprcUpdate,
    HRGN hrgnUpdate,
    UINT flags);

BOOL SetPropW(
    HWND hWnd,
    LPCWSTR lpString,
    HANDLE hData);

BOOL AdjustWindowRectEx(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu,
    DWORD dwExStyle);

BOOL AdjustWindowRectExForDpi(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu,
    DWORD dwExStyle,
    UINT dpi);

BOOL ChangeWindowMessageFilterEx(
    HWND hwnd,
    UINT message,
    DWORD action,
    PCHANGEFILTERSTRUCT pChangeFilterStruct);
}

#define DWM_BB_ENABLE 0x00000001
#define DWM_BB_BLURREGION 0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED 0x00000004

#define GWL_EXSTYLE (-20)

#define LWA_COLORKEY 0x00000001
#define LWA_ALPHA 0x00000002

#define RGB(r, g, b) ((COLORREF) (((BYTE) (r) | ((WORD) ((BYTE) (g)) << 8)) | (((DWORD) (BYTE) (b)) << 16)))

#define RDW_INVALIDATE 0x0001
#define RDW_INTERNALPAINT 0x0002
#define RDW_ERASE 0x0004

#define RDW_VALIDATE 0x0008
#define RDW_NOINTERNALPAINT 0x0010
#define RDW_NOERASE 0x0020

#define RDW_NOCHILDREN 0x0040
#define RDW_ALLCHILDREN 0x0080

#define RDW_UPDATENOW 0x0100
#define RDW_ERASENOW 0x0200

#define RDW_FRAME 0x0400
#define RDW_NOFRAME 0x0800

#define WM_COPYDATA 0x004A

#define WM_DEVICECHANGE 0x0219

#define WM_MDICREATE 0x0220
#define WM_MDIDESTROY 0x0221
#define WM_MDIACTIVATE 0x0222
#define WM_MDIRESTORE 0x0223
#define WM_MDINEXT 0x0224
#define WM_MDIMAXIMIZE 0x0225
#define WM_MDITILE 0x0226
#define WM_MDICASCADE 0x0227
#define WM_MDIICONARRANGE 0x0228
#define WM_MDIGETACTIVE 0x0229

#define WM_MDISETMENU 0x0230
#define WM_ENTERSIZEMOVE 0x0231
#define WM_EXITSIZEMOVE 0x0232
#define WM_DROPFILES 0x0233
#define WM_MDIREFRESHMENU 0x0234
#define WM_POINTERDEVICECHANGE 0x238
#define WM_POINTERDEVICEINRANGE 0x239
#define WM_POINTERDEVICEOUTOFRANGE 0x23a
#define WM_TOUCH 0x0240
#define WM_NCPOINTERUPDATE 0x0241
#define WM_NCPOINTERDOWN 0x0242
#define WM_NCPOINTERUP 0x0243
#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247
#define WM_POINTERENTER 0x0249
#define WM_POINTERLEAVE 0x024a
#define WM_POINTERACTIVATE 0x024b
#define WM_POINTERCAPTURECHANGED 0x024c
#define WM_TOUCHHITTESTING 0x024d
#define WM_POINTERWHEEL 0x024e
#define WM_POINTERHWHEEL 0x024f

#define WM_IME_SETCONTEXT 0x0281
#define WM_IME_NOTIFY 0x0282
#define WM_IME_CONTROL 0x0283
#define WM_IME_COMPOSITIONFULL 0x0284
#define WM_IME_SELECT 0x0285
#define WM_IME_CHAR 0x0286
#define WM_IME_REQUEST 0x0288
#define WM_IME_KEYDOWN 0x0290
#define WM_IME_KEYUP 0x0291

#define WM_MOUSEHOVER 0x02A1
#define WM_MOUSELEAVE 0x02A3
#define WM_NCMOUSEHOVER 0x02A0
#define WM_NCMOUSELEAVE 0x02A2
#define WM_WTSSESSION_CHANGE 0x02B1
#define WM_TABLET_FIRST 0x02c0
#define WM_TABLET_LAST 0x02df
#define WM_CUT 0x0300
#define WM_COPY 0x0301
#define WM_PASTE 0x0302
#define WM_CLEAR 0x0303
#define WM_UNDO 0x0304
#define WM_RENDERFORMAT 0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307
#define WM_DRAWCLIPBOARD 0x0308
#define WM_PAINTCLIPBOARD 0x0309
#define WM_VSCROLLCLIPBOARD 0x030A
#define WM_SIZECLIPBOARD 0x030B
#define WM_ASKCBFORMATNAME 0x030C
#define WM_CHANGECBCHAIN 0x030D
#define WM_HSCROLLCLIPBOARD 0x030E
#define WM_QUERYNEWPALETTE 0x030F
#define WM_PALETTEISCHANGING 0x0310
#define WM_PALETTECHANGED 0x0311
#define WM_HOTKEY 0x0312
#define WM_PRINT 0x0317
#define WM_PRINTCLIENT 0x0318
#define WM_APPCOMMAND 0x0319
#define WM_THEMECHANGED 0x031A
#define WM_WININICHANGE 0x001A
#define WM_SETTINGCHANGE WM_WININICHANGE
#define WM_CLIPBOARDUPDATE 0x031d
#define WM_DWMCOMPOSITIONCHANGED 0x031e
#define WM_DWMNCRENDERINGCHANGED 0x031f
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#define WM_DWMWINDOWMAXIMIZEDCHANGE 0x0321
#define WM_DWMSENDICONICTHUMBNAIL 0x0323
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#define WM_GETTITLEBARINFOEX 0x033f

#define WM_HANDHELDFIRST 0x0358
#define WM_HANDHELDLAST 0x035F
#define WM_AFXFIRST 0x0360
#define WM_AFXLAST 0x037F
#define WM_PENWINFIRST 0x0380
#define WM_PENWINLAST 0x038F
#define WM_APP 0x8000
#define WM_USER 0x0400

#define MSGFLT_RESET (0)
#define MSGFLT_ALLOW (1)
#define MSGFLT_DISALLOW (2)

typedef struct tagWINDOWPLACEMENT {
    UINT length;
    UINT flags;
    UINT showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT rcNormalPosition;
    RECT rcDevice;
} WINDOWPLACEMENT;

extern "C" {
void DragAcceptFiles(
    HWND hWnd,
    BOOL fAccept);

BOOL ClientToScreen(
    HWND hWnd,
    LPPOINT lpPoint);

UINT GetDpiForWindow(
    HWND hwnd);

BOOL SetWindowPlacement(
    HWND hWnd,
    const WINDOWPLACEMENT *lpwndpl);

BOOL GetWindowPlacement(
    HWND hWnd,
    WINDOWPLACEMENT *lpwndpl);
}

#define VK_SHIFT 0x10
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_SNAPSHOT 0x2C

using EXECUTION_STATE = DWORD;

extern "C" {
HWND GetActiveWindow();

HANDLE GetPropW(
    HWND hWnd,
    LPCWSTR lpString);

SHORT GetAsyncKeyState(
    int vKey);

EXECUTION_STATE SetThreadExecutionState(
    EXECUTION_STATE esFlags);

BOOL SystemParametersInfoW(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni);

HANDLE RemovePropW(
    HWND hWnd,
    LPCWSTR lpString);

BOOL DestroyIcon(
    HICON hIcon);

BOOL SetWindowTextW(
    HWND hWnd,
    LPWSTR lpString);

BOOL SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags);

int WINAPI GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount);
int WINAPI GetWindowTextLengthW(HWND hWnd);
BOOL WINAPI GetClientRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI GetWindowRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
}

#define ES_SYSTEM_REQUIRED 0x00000001
#define ES_DISPLAY_REQUIRED 0x00000002
#define ES_USER_PRESENT 0x00000004
#define ES_CONTINUOUS 0x80000000

#define SPI_SETMOUSETRAILS 0x005D
#define SPI_GETMOUSETRAILS 0x005E

#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOREDRAW 0x0008
#define SWP_NOACTIVATE 0x0010
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW 0x0040
#define SWP_HIDEWINDOW 0x0080
#define SWP_NOCOPYBITS 0x0100
#define SWP_NOOWNERZORDER 0x0200
#define SWP_NOSENDCHANGING 0x0400

#define SWP_DRAWFRAME SWP_FRAMECHANGED
#define SWP_NOREPOSITION SWP_NOOWNERZORDER
#define SWP_DEFERERASE 0x2000
#define SWP_ASYNCWINDOWPOS 0x4000

#define HWND_TOP ((HWND) 0)
#define HWND_BOTTOM ((HWND) 1)
#define HWND_TOPMOST ((HWND) -1)
#define HWND_NOTOPMOST ((HWND) -2)

#define GWL_STYLE (-16)

#define BI_BITFIELDS 3

typedef long FXPT2DOT30;

typedef struct tagCIEXYZ {
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct tagICEXYZTRIPLE {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct {
    DWORD bV5Size;
    LONG bV5Width;
    LONG bV5Height;
    WORD bV5Planes;
    WORD bV5BitCount;
    DWORD bV5Compression;
    DWORD bV5SizeImage;
    LONG bV5XPelsPerMeter;
    LONG bV5YPelsPerMeter;
    DWORD bV5ClrUsed;
    DWORD bV5ClrImportant;
    DWORD bV5RedMask;
    DWORD bV5GreenMask;
    DWORD bV5BlueMask;
    DWORD bV5AlphaMask;
    DWORD bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    DWORD bV5GammaRed;
    DWORD bV5GammaGreen;
    DWORD bV5GammaBlue;
    DWORD bV5Intent;
    DWORD bV5ProfileData;
    DWORD bV5ProfileSize;
    DWORD bV5Reserved;
} BITMAPV5HEADER, *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

typedef struct _ICONINFO {
    BOOL fIcon;
    DWORD xHotspot;
    DWORD yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
} ICONINFO;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

using PICONINFO = ICONINFO *;

extern "C" {
HBITMAP CreateDIBSection(
    HDC hdc,
    const BITMAPINFO *pbmi,
    UINT usage,
    VOID **ppvBits,
    HANDLE hSection,
    DWORD offset);

HICON CreateIconIndirect(
    PICONINFO piconinfo);

HBITMAP CreateBitmap(
    int nWidth,
    int nHeight,
    UINT nPlanes,
    UINT nBitCount,
    const VOID *lpBits);

int GetSystemMetrics(
    int nIndex);

ULONG_PTR GetClassLongPtrW(
    HWND hWnd,
    int nIndex);

BOOL ClipCursor(
    const RECT *lpRect);

HCURSOR SetCursor(
    HCURSOR hCursor);
}

#define WM_SETICON 0x0080

#define DIB_RGB_COLORS 0

#define SM_CXICON 11
#define SM_CYICON 12

#define SM_CXSMICON 49
#define SM_CYSMICON 50

#define GCLP_HICON (-14)
#define GCLP_HICONSM (-34)

#define ICON_SMALL 0
#define ICON_BIG 1
#define ICON_SMALL2 2

typedef struct tagRAWINPUTDEVICE {
    USHORT usUsagePage;
    USHORT usUsage;
    DWORD dwFlags;
    HWND hwndTarget;
} RAWINPUTDEVICE, *PRAWINPUTDEVICE, *LPRAWINPUTDEVICE;

extern "C" {
BOOL RegisterRawInputDevices(
    const RAWINPUTDEVICE *pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize);

BOOL GetCursorPos(
    LPPOINT lpPoint);

BOOL ScreenToClient(
    HWND hWnd,
    LPPOINT lpPoint);

BOOL SetCursorPos(
    int X,
    int Y);

BOOL MoveWindow(
    HWND hWnd,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    BOOL bRepaint);

BOOL SetRect(
    LPRECT lprc,
    int xLeft,
    int yTop,
    int xRight,
    int yBottom);

BOOL GetLayeredWindowAttributes(
    HWND hwnd,
    COLORREF *pcrKey,
    BYTE *pbAlpha,
    DWORD *pdwFlags);

HWND WindowFromPoint(
    POINT Point);

BOOL PtInRect(
    const RECT *lprc,
    POINT pt);

BOOL BringWindowToTop(
    HWND hWnd);

BOOL SetForegroundWindow(
    HWND hWnd);

HWND SetFocus(
    HWND hWnd);

BOOL FlashWindow(
    HWND hWnd,
    BOOL bInvert);
}

#define RIDEV_REMOVE 0x00000001
#define RIDEV_EXCLUDE 0x00000010
#define RIDEV_PAGEONLY 0x00000020
#define RIDEV_NOLEGACY 0x00000030
#define RIDEV_INPUTSINK 0x00000100
#define RIDEV_CAPTUREMOUSE 0x00000200
#define RIDEV_NOHOTKEYS 0x00000200
#define RIDEV_APPKEYS 0x00000400
#define RIDEV_EXINPUTSINK 0x00001000
#define RIDEV_DEVNOTIFY 0x00002000

#define WMSZ_LEFT 1
#define WMSZ_RIGHT 2
#define WMSZ_TOP 3
#define WMSZ_TOPLEFT 4
#define WMSZ_TOPRIGHT 5
#define WMSZ_BOTTOM 6
#define WMSZ_BOTTOMLEFT 7
#define WMSZ_BOTTOMRIGHT 8

#define WM_NCCREATE 0x0081
#define WM_NCCALCSIZE 0x0083
#define WM_NCHITTEST 0x0084
#define WM_NCPAINT 0x0085
#define WM_NCACTIVATE 0x0086
#define WM_GETDLGCODE 0x0087

#define WM_DISPLAYCHANGE 0x007E
#define WM_MOUSEACTIVATE 0x0021

#define WM_SIZING 0x0214
#define WM_CAPTURECHANGED 0x0215
#define WM_MOVING 0x0216

#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_ENABLE 0x000A

#define WM_SETREDRAW 0x000B
#define WM_SETTEXT 0x000C
#define WM_GETTEXT 0x000D
#define WM_GETTEXTLENGTH 0x000E
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010

#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_XBUTTONDBLCLK 0x020D
#define WM_MOUSEHWHEEL 0x020e

typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

extern "C" {
BOOL EnableNonClientDpiScaling(
    HWND hwnd);

LRESULT SendMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

HWND SetCapture(
    HWND hWnd);

BOOL ReleaseCapture();

BOOL TrackMouseEvent(
    LPTRACKMOUSEEVENT lpEventTrack);

UINT GetRawInputData(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader);
}

#define XBUTTON1 0x0001
#define XBUTTON2 0x0002

#define GET_WPARAM(wp, lp) (wp)
#define GET_LPARAM(wp, lp) (lp)

#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))

#define TME_HOVER 0x00000001
#define TME_LEAVE 0x00000002
#define TME_NONCLIENT 0x00000010
#define TME_QUERY 0x40000000
#define TME_CANCEL 0x80000000

#define WM_INPUT 0x00FF

typedef struct tagRAWINPUTHEADER {
    DWORD dwType;
    DWORD dwSize;
    HANDLE hDevice;
    WPARAM wParam;
} RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

typedef struct tagRAWKEYBOARD {
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    USHORT VKey;
    UINT Message;
    ULONG ExtraInformation;
} RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;

typedef struct tagRAWMOUSE {
    USHORT usFlags;

    union {
        ULONG ulButtons;

        struct {
            USHORT usButtonFlags;
            USHORT usButtonData;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    ULONG ulRawButtons;
    LONG lLastX;
    LONG lLastY;
    ULONG ulExtraInformation;
} RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;

typedef struct tagRAWHID {
    DWORD dwSizeHid;
    DWORD dwCount;
    BYTE bRawData[1];
} RAWHID, *PRAWHID, *LPRAWHID;

typedef struct tagRAWINPUT {
    RAWINPUTHEADER header;

    union {
        RAWMOUSE mouse;
        RAWKEYBOARD keyboard;
        RAWHID hid;
    } data;
} RAWINPUT, *PRAWINPUT, *LPRAWINPUT;

#define WHEEL_DELTA 120
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short) HIWORD(wParam))

#define RID_INPUT 0x10000003
#define RID_HEADER 0x10000005

#define MOUSE_MOVE_RELATIVE 0
#define MOUSE_MOVE_ABSOLUTE 1

#define WM_ENTERMENULOOP 0x0211
#define WM_EXITMENULOOP 0x0212
#define WM_NEXTMENU 0x0213
#define WM_GETMINMAXINFO 0x0024

typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE, *PSIZE, *LPSIZE;

#define WM_SETCURSOR 0x0020

typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, *NPCOMPOSITIONFORM, *LPCOMPOSITIONFORM;

typedef DWORD HIMC;
typedef DWORD HIMCC;

#define SM_CXMINTRACK 34
#define SM_CYMINTRACK 35
#define SM_CXMAXTRACK 59
#define SM_CYMAXTRACK 60

typedef struct tagWINDOWINFO {
    DWORD cbSize;
    RECT rcWindow;
    RECT rcClient;
    DWORD dwStyle;
    DWORD dwExStyle;
    DWORD dwWindowStatus;
    UINT cxWindowBorders;
    UINT cyWindowBorders;
    ATOM atomWindowType;
    WORD wCreatorVersion;
} WINDOWINFO, *PWINDOWINFO, *LPWINDOWINFO;

typedef struct tagCREATESTRUCTW {
    LPVOID lpCreateParams;
    HINSTANCE hInstance;
    HMENU hMenu;
    HWND hwndParent;
    int cy;
    int cx;
    int y;
    int x;
    LONG style;
    LPCWSTR lpszName;
    LPCWSTR lpszClass;
    DWORD dwExStyle;
} CREATESTRUCTW, *LPCREATESTRUCTW;

#define VK_RETURN 0x0D
#define VK_ESCAPE 0X1B
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28

#define SPI_GETWORKAREA 0x0030
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1

extern "C" {
LONG_PTR GetWindowLongPtrW(
    HWND hWnd,
    int nIndex);

BOOL OffsetRect(
    LPRECT lprc,
    int dx,
    int dy);

HLOCAL LocalAlloc(
    UINT uFlags,
    SIZE_T uBytes);

int MapWindowPoints(
    HWND hWndFrom,
    HWND hWndTo,
    LPPOINT lpPoints,
    UINT cPoints);

HWND GetCapture();

BOOL DragQueryPoint(
    HDROP hDrop,
    POINT *ppt);

UINT DragQueryFileW(
    HDROP hDrop,
    UINT iFile,
    LPWSTR lpszFile,
    UINT cch);

void DragFinish(
    HDROP hDrop);

BOOL DestroyCursor(
    HCURSOR hCursor);

HIMC ImmGetContext(
    HWND Arg1);

BOOL ImmSetCompositionWindow(
    HIMC,
    LPCOMPOSITIONFORM lpCompForm);

BOOL ImmReleaseContext(
    HWND,
    HIMC);

HWND GetParent(
    HWND hWnd);

HMENU GetMenu(
    HWND hWnd);

BOOL GetWindowInfo(
    HWND hwnd,
    PWINDOWINFO pwi);

BOOL IsWindowVisible(
    HWND hWnd);
}

#define CFS_FORCE_POSITION 32
