// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <SDKDDKVer.h>

#include <Windows.h>

#include <Comdef.h>
#include <Jobapi2.h>
#include <Msi.h>
#include <MsiQuery.h>
#include <Ntsecapi.h>
#include <Objbase.h>
#include <Rpc.h>
#include <Shlobj.h>
#include <TlHelp32.h>
#include <WinNT.h>
#include <bcrypt.h>
#include <compressapi.h>
#include <cryptuiapi.h>
#include <dpapi.h>
#include <guiddef.h>
#include <iphlpapi.h>
#include <iptypes.h>
#include <memoryapi.h>
#include <netlistmgr.h>
#include <pathcch.h>
#include <process.h>
#include <psapi.h>
#include <sddl.h>
#include <shellscalingapi.h>
#include <shlwapi.h>
#include <taskschd.h>
#include <threadpoolapiset.h>
#include <wincodec.h>
#include <wincrypt.h>
#include <windowsx.h>
#include <winnt.h>
#include <winsock2.h>
#include <winuser.h>
#include <wlanapi.h>
#include <wrl/client.h>
#include <wtypes.h>
// @see https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
#define SCHANNEL_USE_BLACKLISTS
#include <Schnlsp.h>
#include <schannel.h>
// Must be defined for security.h
// @see https://stackoverflow.com/questions/11561475/sspi-header-file-fatal-error
#define SECURITY_WIN32
#include <security.h>
#include <wincred.h>
#include <winsafer.h>

export module stormkit.core.win32;

import std;

namespace stdr = std::ranges;

export namespace win32 {
    using ::__fastfail;
    using ::_beginthreadex;
    using ::_get_errno;
    using ::ACCESS_MASK;
    using ::AcquireSRWLockExclusive;
    using ::AcquireSRWLockShared;
    using ::AddSIDToBoundaryDescriptor;
    using ::AdjustTokenPrivileges;
    using ::AdjustWindowRect;
    using ::AdjustWindowRectEx;
    using ::AllocateAndInitializeSid;
    using ::AssignProcessToJobObject;
    using ::BCRYPT_ALG_HANDLE;
    using ::BCRYPT_KEY_HANDLE;
    using ::BCryptCloseAlgorithmProvider;
    using ::BCryptDecrypt;
    using ::BCryptDestroyKey;
    using ::BCryptEncrypt;
    using ::BCryptGenerateSymmetricKey;
    using ::BCryptGetProperty;
    using ::BCryptOpenAlgorithmProvider;
    using ::BCryptSetProperty;
    using ::BOOL;
    using ::BOOLEAN;
    using ::BYTE;
    using ::CancelIo;
    using ::CancelIoEx;
    using ::CancelWaitableTimer;
    using ::CERT_CHAIN_CONTEXT;
    using ::CERT_CHAIN_FIND_BY_ISSUER_PARA;
    using ::CERT_CHAIN_PARA;
    using ::CERT_CHAIN_POLICY_PARA;
    using ::CERT_CHAIN_POLICY_STATUS;
    using ::CERT_CONTEXT;
    using ::CERT_ENHKEY_USAGE;
    using ::CERT_NAME_BLOB;
    using ::CERT_SIMPLE_CHAIN;
    using ::CERT_USAGE_MATCH;
    using ::CertAddCertificateContextToStore;
    using ::CertCloseStore;
    using ::CertDeleteCertificateFromStore;
    using ::CertDuplicateCertificateChain;
    using ::CertDuplicateCertificateContext;
    using ::CertDuplicateStore;
    using ::CertEnumCertificatesInStore;
    using ::CertFindCertificateInStore;
    using ::CertFindChainInStore;
    using ::CertFreeCertificateChain;
    using ::CertFreeCertificateContext;
    using ::CertGetCertificateChain;
    using ::CertGetCertificateContextProperty;
    using ::CertGetPublicKeyLength;
    using ::CertGetStoreProperty;
    using ::CertNameToStrA;
    using ::CertNameToStrW;
    using ::CertOpenStore;
    using ::CertOpenSystemStoreA;
    using ::CertOpenSystemStoreW;
    using ::CertStrToNameA;
    using ::CertStrToNameW;
    using ::CertVerifyCertificateChainPolicy;
    using ::CertVerifyTimeValidity;
    using ::ChangeTimerQueueTimer;
    using ::CheckTokenMembership;
    using ::CloseClipboard;
    using ::CloseCompressor;
    using ::CloseDecompressor;
    using ::CloseHandle;
    using ::ClosePrivateNamespace;
    using ::CloseServiceHandle;
    using ::CloseThreadpool;
    using ::CLSID_NetworkListManager;
    using ::CLSID_WICImagingFactory2;
    using ::CoCreateGuid;
    using ::CoCreateInstance;
    using ::COINIT;
    using ::CoInitializeEx;
    using ::CoInitializeSecurity;
    using ::COLORREF;
    using ::CompareStringOrdinal;
    using ::Compress;
    using ::COMPRESSOR_HANDLE;
    using ::COMPUTER_NAME_FORMAT;
    using ::ConnectNamedPipe;
    using ::ControlServiceExA;
    using ::ControlServiceExW;
    using ::ConvertSidToStringSidA;
    using ::ConvertSidToStringSidW;
    using ::ConvertStringSecurityDescriptorToSecurityDescriptorA;
    using ::ConvertStringSecurityDescriptorToSecurityDescriptorW;
    using ::ConvertStringSidToSidA;
    using ::ConvertStringSidToSidW;
    using ::CopySid;
    using ::CoUninitialize;
    using ::CreateBoundaryDescriptorA;
    using ::CreateBoundaryDescriptorW;
    using ::CreateCompressor;
    using ::CreateDecompressor;
    using ::CreateDirectoryA;
    using ::CreateDirectoryW;
    using ::CreateEventA;
    using ::CreateEventW;
    using ::CreateFileA;
    using ::CreateFileMappingA;
    using ::CreateFileMappingW;
    using ::CreateFileW;
    using ::CreateIoCompletionPort;
    using ::CreateJobObjectA;
    using ::CreateJobObjectW;
    using ::CreateMailslotA;
    using ::CreateMailslotW;
    using ::CreateMutexA;
    using ::CreateMutexW;
    using ::CreateNamedPipeA;
    using ::CreateNamedPipeW;
    using ::CreatePipe;
    using ::CreatePrivateNamespaceA;
    using ::CreatePrivateNamespaceW;
    using ::CreateProcessA;
    using ::CreateProcessW;
    using ::CreateSemaphoreA;
    using ::CreateSemaphoreW;
    using ::CreateSolidBrush;
    using ::CreateThreadpool;
    using ::CreateThreadpoolWork;
    using ::CreateTimerQueue;
    using ::CreateTimerQueueTimer;
    using ::CreateToolhelp32Snapshot;
    using ::CreateWaitableTimerA;
    using ::CreateWaitableTimerW;
    using ::CreateWellKnownSid;
    using ::CreateWindowExA;
    using ::CreateWindowExW;
    using ::CRITICAL_SECTION;
    using ::CryptBinaryToStringA;
    using ::CryptBinaryToStringW;
    using ::CryptProtectData;
    using ::CryptProtectMemory;
    using ::CryptStringToBinaryA;
    using ::CryptStringToBinaryW;
    using ::CRYPTUI_WIZ_IMPORT_SRC_INFO;
    using ::CryptUIWizImport;
    using ::CryptUnprotectData;
    using ::CryptUnprotectMemory;
    using ::DATA_BLOB;
    using ::DecodePointer;
    using ::Decompress;
    using ::DECOMPRESSOR_HANDLE;
    using ::DefSubclassProc;
    using ::DefWindowProcA;
    using ::DefWindowProcW;
    using ::DeleteBoundaryDescriptor;
    using ::DeleteCriticalSection;
    using ::DeleteObject;
    using ::DeleteService;
    using ::DeleteSynchronizationBarrier;
    using ::DeleteTimerQueueEx;
    using ::DeleteTimerQueueTimer;
    using ::DestroyThreadpoolEnvironment;
    using ::DestroyWindow;
    using ::DisconnectNamedPipe;
    using ::DuplicateHandle;
    using ::DuplicateTokenEx;
    using ::DWORD;
    using ::DWORD_PTR;
    using ::EmptyClipboard;
    using ::EncodePointer;
    using ::EnterCriticalSection;
    using ::EnterSynchronizationBarrier;
    using ::EOLE_AUTHENTICATION_CAPABILITIES;
    using ::EqualSid;
    using ::FILETIME;
    using ::FileTimeToSystemTime;
    using ::FillRect;
    using ::FlushFileBuffers;
    using ::FormatMessageA;
    using ::FormatMessageW;
    using ::FreeLibrary;
    using ::FreeSid;
    using ::GetClassInfoA;
    using ::GetClassInfoW;
    using ::GetClipboardData;
    using ::GetComputerNameExA;
    using ::GetComputerNameExW;
    using ::GetCurrentProcess;
    using ::GetCurrentProcessToken;
    using ::GetCurrentThread;
    using ::GetCurrentThreadId;
    using ::GetDateFormatEx;
    using ::GetExitCodeProcess;
    using ::GetExitCodeThread;
    using ::GetFileSize;
    using ::GetFileSizeEx;
    using ::GetFileType;
    using ::GetFileVersionInfoA;
    using ::GetFileVersionInfoSizeA;
    using ::GetFileVersionInfoSizeW;
    using ::GetFileVersionInfoW;
    using ::GetFinalPathNameByHandleA;
    using ::GetFinalPathNameByHandleW;
    using ::GetHandleInformation;
    using ::GetLastError;
    using ::GetLengthSid;
    using ::GetLogicalProcessorInformationEx;
    using ::GetModuleFileNameA;
    using ::GetModuleFileNameW;
    using ::GetModuleHandleA;
    using ::GetModuleHandleW;
    using ::GetPhysicallyInstalledSystemMemory;
    using ::GetProcAddress;
    using ::GetProcessHandleCount;
    using ::GetProcessHeap;
    using ::GetProcessId;
    using ::GetProcessTimes;
    using ::GetQueuedCompletionStatus;
    using ::GetSecurityDescriptorControl;
    using ::GetSidIdentifierAuthority;
    using ::GetSidLengthRequired;
    using ::GetSidSubAuthority;
    using ::GetSidSubAuthorityCount;
    using ::GetStdHandle;
    using ::GetSystemInfo;
    using ::GetSystemTime;
    using ::GetSystemTimeAdjustment;
    using ::GetSystemTimeAsFileTime;
    using ::GetThreadDescription;
    using ::GetThreadId;
    using ::GetTickCount64;
    using ::GetTimeFormatEx;
    using ::GetTimeZoneInformation;
    using ::GetTokenInformation;
    using ::GetWindowLongA;
    using ::GetWindowLongW;
    using ::GetWindowTextA;
    using ::GetWindowTextLengthA;
    using ::GetWindowTextLengthW;
    using ::GetWindowTextW;
    using ::GlobalAlloc;
    using ::GlobalFree;
    using ::GlobalLock;
    using ::GlobalMemoryStatusEx;
    using ::GlobalUnlock;
    using ::GUID;
    using ::HANDLE;
    using ::HBRUSH;
    using ::HCERTSTORE;
    using ::HeapAlloc;
    using ::HeapCompact;
    using ::HeapCreate;
    using ::HeapDestroy;
    using ::HeapFree;
    using ::HeapLock;
    using ::HeapUnlock;
    using ::HeapValidate;
    using ::HFONT;
    using ::HGLOBAL;
    using ::HINSTANCE__;
    using ::HMENU;
    using ::HMODULE;
    using ::HRESULT;
    using ::HWND;
    using ::IID_INetworkListManager;
    using ::IIDFromString;
    using ::ImpersonateLoggedOnUser;
    using ::INetworkListManager;
    using ::InitializeCriticalSection;
    using ::InitializeCriticalSectionAndSpinCount;
    using ::InitializeCriticalSectionEx;
    using ::InitializeSid;
    using ::InitializeSListHead;
    using ::InitializeSRWLock;
    using ::InitializeSynchronizationBarrier;
    using ::InitializeThreadpoolEnvironment;
    using ::InterlockedFlushSList;
    using ::InterlockedPopEntrySList;
    using ::InterlockedPushEntrySList;
    using ::InvalidateRect;
    using ::IsClipboardFormatAvailable;
    using ::IsEqualGUID;
    using ::IsValidSid;
    using ::IWICBitmapDecoder;
    using ::IWICFormatConverter;
    using ::IWICImagingFactory;
    using ::JOBOBJECT_EXTENDED_LIMIT_INFORMATION;
    using ::JOBOBJECTINFOCLASS;
    using ::K32EnumDeviceDrivers;
    using ::K32EnumProcesses;
    using ::K32GetModuleFileNameExA;
    using ::K32GetModuleFileNameExW;
    using ::LARGE_INTEGER;
    using ::LeaveCriticalSection;
    using ::LoadLibraryA;
    using ::LoadLibraryExA;
    using ::LoadLibraryExW;
    using ::LoadLibraryW;
    using ::LocalFree;
    using ::LockFile;
    using ::LockFileEx;
    using ::LOGICAL_PROCESSOR_RELATIONSHIP;
    using ::LONG;
    using ::LookupAccountSidA;
    using ::LookupAccountSidW;
    using ::LookupPrivilegeNameA;
    using ::LookupPrivilegeNameW;
    using ::LookupPrivilegeValueA;
    using ::LookupPrivilegeValueW;
    using ::LPARAM;
    using ::LPBYTE;
    using ::LPCSTR;
    using ::LPCWSTR;
    using ::LPDWORD;
    using ::LPOVERLAPPED;
    using ::LPSTR;
    using ::LPVOID;
    using ::LPWSTR;
    using ::LRESULT;
    using ::LSA_HANDLE;
    using ::LSA_OBJECT_ATTRIBUTES;
    using ::LSA_UNICODE_STRING;
    using ::LsaAddAccountRights;
    using ::LsaClose;
    using ::LsaNtStatusToWinError;
    using ::LsaOpenPolicy;
    using ::LsaRemoveAccountRights;
    using ::LSTATUS;
    using ::LUID;
    using ::LUID_AND_ATTRIBUTES;
    using ::MapViewOfFile;
    using ::MEMORYSTATUSEX;
    using ::MonitorFromWindow;
    using ::MoveFileExA;
    using ::MoveFileExW;
    using ::MsiCloseHandle;
    using ::MsiDatabaseOpenViewA;
    using ::MsiDatabaseOpenViewW;
    using ::MsiEnumProductsExA;
    using ::MsiEnumProductsExW;
    using ::MsiGetProductInfoExA;
    using ::MsiGetProductInfoExW;
    using ::MSIHANDLE;
    using ::MSIINSTALLCONTEXT;
    using ::MsiIsProductElevatedA;
    using ::MsiIsProductElevatedW;
    using ::MsiOpenDatabaseA;
    using ::MsiOpenDatabaseW;
    using ::MsiOpenPackageA;
    using ::MsiOpenPackageW;
    using ::MsiRecordGetStringA;
    using ::MsiRecordGetStringW;
    using ::MsiViewExecute;
    using ::MsiViewFetch;
    using ::MultiByteToWideChar;
    using ::NLM_CONNECTIVITY;
    using ::NTSTATUS;
    using ::OpenClipboard;
    using ::OpenEventA;
    using ::OpenEventW;
    using ::OpenFileMappingA;
    using ::OpenFileMappingW;
    using ::OpenJobObjectA;
    using ::OpenJobObjectW;
    using ::OpenMutexA;
    using ::OpenMutexW;
    using ::OpenPrivateNamespaceA;
    using ::OpenPrivateNamespaceW;
    using ::OpenProcess;
    using ::OpenProcessToken;
    using ::OpenSCManagerA;
    using ::OpenSCManagerW;
    using ::OpenSemaphoreA;
    using ::OpenSemaphoreW;
    using ::OpenServiceA;
    using ::OpenServiceW;
    using ::OpenWaitableTimerA;
    using ::OpenWaitableTimerW;
    using ::OVERLAPPED;
    using ::PAPCFUNC;
    using ::PathCchRemoveFileSpec;
    using ::PBYTE;
    using ::PCCERT_CHAIN_CONTEXT;
    using ::PCCERT_CONTEXT;
    using ::PCSTR;
    using ::PCWSTR;
    using ::PDWORD;
    using ::PeekNamedPipe;
    using ::PHANDLE;
    using ::PLARGE_INTEGER;
    using ::PMSIHANDLE;
    using ::POINT;
    using ::PRIVILEGE_SET;
    using ::PrivilegeCheck;
    using ::Process32First;
    using ::Process32FirstW;
    using ::Process32Next;
    using ::Process32NextW;
    using ::PROCESS_INFORMATION;
    using ::PROCESSENTRY32;
    using ::PROCESSENTRY32W;
    using ::ProcessIdToSessionId;
    using ::PSECURITY_DESCRIPTOR;
    using ::PSID;
    using ::PSID_IDENTIFIER_AUTHORITY;
    using ::PSID_NAME_USE;
    using ::PSIZE_T;
    using ::PSLIST_ENTRY;
    using ::PSLIST_HEADER;
    using ::PTIMERAPCROUTINE;
    using ::PTOKEN_GROUPS;
    using ::PTP_CALLBACK_INSTANCE;
    using ::PTP_WORK;
    using ::PUCHAR;
    using ::PULONG;
    using ::PUNICODE_STRING;
    using ::PVOID;
    using ::PWSTR;
    using ::QueryDepthSList;
    using ::QueryServiceConfigA;
    using ::QueryServiceConfigW;
    using ::QueryServiceStatusEx;
    using ::QueueUserAPC;
    using ::RaiseException;
    using ::ReadFile;
    using ::RECT;
    using ::RegisterClassA;
    using ::RegisterClassW;
    using ::ReleaseMutex;
    using ::ReleaseSemaphore;
    using ::ReleaseSRWLockExclusive;
    using ::ReleaseSRWLockShared;
    using ::ResetCompressor;
    using ::ResetDecompressor;
    using ::ResetEvent;
    using ::ResumeThread;
    using ::RevertToSelf;
    using ::RPC_STATUS;
    using ::RPC_WSTR;
    using ::RtlSecureZeroMemory;
    using ::SC_HANDLE;
    using ::ScreenToClient;
    using ::SECURITY_ATTRIBUTES;
    using ::SECURITY_DESCRIPTOR;
    using ::SECURITY_DESCRIPTOR_CONTROL;
    using ::SECURITY_IMPERSONATION_LEVEL;
    using ::SendMessageA;
    using ::SendMessageW;
    using ::SetClipboardData;
    using ::SetEndOfFile;
    using ::SetEvent;
    using ::SetFilePointer;
    using ::SetFilePointerEx;
    using ::SetFocus;
    using ::SetHandleInformation;
    using ::SetInformationJobObject;
    using ::SetNamedPipeHandleState;
    using ::SetProcessDpiAwarenessContext;
    using ::SetThreadDescription;
    using ::SetThreadDpiAwarenessContext;
    using ::SetThreadpoolCallbackPool;
    using ::SetThreadpoolCallbackRunsLong;
    using ::SetThreadpoolThreadMaximum;
    using ::SetThreadpoolThreadMinimum;
    using ::SetTokenInformation;
    using ::SetWaitableTimer;
    using ::SetWindowSubclass;
    using ::SetWindowTextA;
    using ::SetWindowTextW;
    using ::SHDeleteKeyA;
    using ::SHDeleteKeyW;
    using ::ShowWindow;
    using ::SID;
    using ::SID_AND_ATTRIBUTES;
    using ::SID_IDENTIFIER_AUTHORITY;
    using ::SID_NAME_USE;
    using ::SIZE_T;
    using ::Sleep;
    using ::SLIST_ENTRY;
    using ::SLIST_HEADER;
    using ::SRWLOCK;
    using ::StartServiceA;
    using ::StartServiceW;
    using ::STARTUPINFO;
    using ::StringFromGUID2;
    using ::SubmitThreadpoolWork;
    using ::SuspendThread;
    using ::SYNCHRONIZATION_BARRIER;
    using ::SYSTEM_INFO;
    using ::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;
    using ::SYSTEMTIME;
    using ::SystemTimeToFileTime;
    using ::TerminateThread;
    using ::TIME_ZONE_INFORMATION;
    using ::timeval;
    using ::TOKEN_GROUPS;
    using ::TOKEN_INFORMATION_CLASS;
    using ::TOKEN_MANDATORY_LABEL;
    using ::TOKEN_PRIVILEGES;
    using ::TOKEN_STATISTICS;
    using ::TOKEN_TYPE;
    using ::TP_CALLBACK_ENVIRON;
    using ::TP_POOL;
    using ::TryAcquireSRWLockExclusive;
    using ::TryAcquireSRWLockShared;
    using ::UCHAR;
    using ::UINT;
    using ::UINT_PTR;
    using ::ULARGE_INTEGER;
    using ::ULONG;
    using ::ULONG_PTR;
    using ::UNICODE_STRING;
    using ::UnlockFile;
    using ::UnlockFileEx;
    using ::UnmapViewOfFile;
    using ::UnregisterClassA;
    using ::UnregisterClassW;
    using ::UpdateWindow;
    using ::USHORT;
    using ::UuidFromStringA;
    using ::UuidFromStringW;
    using ::UuidIsNil;
    using ::VerQueryValueA;
    using ::VerQueryValueW;
    using ::VS_FIXEDFILEINFO;
    using ::WaitForMultipleObjects;
    using ::WaitForMultipleObjectsEx;
    using ::WaitForSingleObject;
    using ::WaitForSingleObjectEx;
    using ::WaitNamedPipeA;
    using ::WaitNamedPipeW;
    using ::WaitOnAddress;
    using ::WAITORTIMERCALLBACK;
    using ::WakeByAddressAll;
    using ::WakeByAddressSingle;
    using ::WICDecodeOptions;
    using ::WideCharToMultiByte;
    using ::WNDCLASSA;
    using ::WPARAM;
    using ::WriteFile;

    auto MakeHRESULT(LONG severity, LONG facility, LONG code) noexcept -> HRESULT;
    auto Code(HRESULT) noexcept -> bool;
    auto Severity(HRESULT) noexcept -> bool;
    auto Facility(HRESULT) noexcept -> bool;
    auto Succeeded(HRESULT) noexcept -> bool;
    auto Failed(HRESULT) noexcept -> bool;

    auto GetXLPARAM(LPARAM) noexcept -> int;
    auto GetYLPARAM(LPARAM) noexcept -> int;

    auto GetAppCommandLPARAM(LPARAM) noexcept -> int;
    auto GetDeviceLPARAM(LPARAM) noexcept -> int;
    auto GetFlagsLPARAM(LPARAM) noexcept -> int;
    auto GetKeyStateLPARAM(LPARAM) noexcept -> int;
    auto GetKeyStateWPARAM(WPARAM) noexcept -> int;
    auto GetRawInputCodeWPARAM(WPARAM) noexcept -> int;
    auto GetNCHittestWPARAM(WPARAM) noexcept -> int;
    auto GetWheelDeltaWPARAM(WPARAM) noexcept -> int;
    auto GetXButtonWPARAM(WPARAM) noexcept -> int;

    auto Rgb(BYTE, BYTE, BYTE) noexcept -> COLORREF;

#pragma push_macro("INVALID_HANDLE_VALUE")
#undef INVALID_HANDLE_VALUE
    inline const /*constexpr*/ auto INVALID_HANDLE_VALUE =
#pragma pop_macro("INVALID_HANDLE_VALUE")
      INVALID_HANDLE_VALUE;

#pragma push_macro("FALSE")
#undef FALSE
    inline constexpr auto FALSE
#pragma pop_macro("FALSE")
      = FALSE;

#pragma push_macro("TRUE")
#undef TRUE
    inline constexpr auto TRUE =
#pragma pop_macro("TRUE")
      TRUE;

#pragma push_macro("PAGE_EXECUTE")
#undef PAGE_EXECUTE
    inline constexpr auto PAGE_EXECUTE =
#pragma pop_macro("PAGE_EXECUTE")
      PAGE_EXECUTE;
#pragma push_macro("PAGE_EXECUTE_READ")
#undef PAGE_EXECUTE_READ
    inline constexpr auto PAGE_EXECUTE_READ =
#pragma pop_macro("PAGE_EXECUTE_READ")
      PAGE_EXECUTE_READ;
#pragma push_macro("PAGE_EXECUTE_READWRITE")
#undef PAGE_EXECUTE_READWRITE
    inline constexpr auto PAGE_EXECUTE_READWRITE =
#pragma pop_macro("PAGE_EXECUTE_READWRITE")
      PAGE_EXECUTE_READWRITE;
#pragma push_macro("PAGE_EXECUTE_WRITECOPY")
#undef PAGE_EXECUTE_WRITECOPY
    inline constexpr auto PAGE_EXECUTE_WRITECOPY =
#pragma pop_macro("PAGE_EXECUTE_WRITECOPY")
      PAGE_EXECUTE_WRITECOPY;
#pragma push_macro("PAGE_NOACCESS")
#undef PAGE_NOACCESS
    inline constexpr auto PAGE_NOACCESS =
#pragma pop_macro("PAGE_NOACCESS")
      PAGE_NOACCESS;
#pragma push_macro("PAGE_READONLY")
#undef PAGE_READONLY
    inline constexpr auto PAGE_READONLY =
#pragma pop_macro("PAGE_READONLY")
      PAGE_READONLY;
#pragma push_macro("PAGE_READWRITE")
#undef PAGE_READWRITE
    inline constexpr auto PAGE_READWRITE =
#pragma pop_macro("PAGE_READWRITE")
      PAGE_READWRITE;
#pragma push_macro("PAGE_WRITECOPY")
#undef PAGE_WRITECOPY
    inline constexpr auto PAGE_WRITECOPY =
#pragma pop_macro("PAGE_WRITECOPY")
      PAGE_WRITECOPY;
#pragma push_macro("PAGE_TARGETS_INVALID")
#undef PAGE_TARGETS_INVALID
    inline constexpr auto PAGE_TARGETS_INVALID =
#pragma pop_macro("PAGE_TARGETS_INVALID")
      PAGE_TARGETS_INVALID;
#pragma push_macro("PAGE_TARGETS_NO_UPDATE")
#undef PAGE_TARGETS_NO_UPDATE
    inline constexpr auto PAGE_TARGETS_NO_UPDATE =
#pragma pop_macro("PAGE_TARGETS_NO_UPDATE")
      PAGE_TARGETS_NO_UPDATE;

#pragma push_macro("PAGE_GUARD")
#undef PAGE_GUARD
    inline constexpr auto PAGE_GUARD =
#pragma pop_macro("PAGE_GUARD")
      PAGE_GUARD;
#pragma push_macro("PAGE_NOCACHE")
#undef PAGE_NOCACHE
    inline constexpr auto PAGE_NOCACHE =
#pragma pop_macro("PAGE_NOCACHE")
      PAGE_NOCACHE;
#pragma push_macro("PAGE_WRITECOMBINE")
#undef PAGE_WRITECOMBINE
    inline constexpr auto PAGE_WRITECOMBINE =
#pragma pop_macro("PAGE_WRITECOMBINE")
      PAGE_WRITECOMBINE;

#pragma push_macro("GENERIC_ALL")
#undef GENERIC_ALL
    inline constexpr auto GENERIC_ALL =
#pragma pop_macro("GENERIC_ALL")
      GENERIC_ALL;
#pragma push_macro("GENERIC_EXECUTE")
#undef GENERIC_EXECUTE
    inline constexpr auto GENERIC_EXECUTE =
#pragma pop_macro("GENERIC_EXECUTE")
      GENERIC_EXECUTE;
#pragma push_macro("GENERIC_WRITE")
#undef GENERIC_WRITE
    inline constexpr auto GENERIC_WRITE =
#pragma pop_macro("GENERIC_WRITE")
      GENERIC_WRITE;
#pragma push_macro("GENERIC_READ")
#undef GENERIC_READ
    inline constexpr auto GENERIC_READ =
#pragma pop_macro("GENERIC_READ")
      GENERIC_READ;

#pragma push_macro("CREATE_ALWAYS")
#undef CREATE_ALWAYS
    inline constexpr auto CREATE_ALWAYS =
#pragma pop_macro("CREATE_ALWAYS")
      CREATE_ALWAYS;
#pragma push_macro("CREATE_NEW")
#undef CREATE_NEW
    inline constexpr auto CREATE_NEW =
#pragma pop_macro("CREATE_NEW")
      CREATE_NEW;
#pragma push_macro("OPEN_ALWAYS")
#undef OPEN_ALWAYS
    inline constexpr auto OPEN_ALWAYS =
#pragma pop_macro("OPEN_ALWAYS")
      OPEN_ALWAYS;
#pragma push_macro("OPEN_EXISTING")
#undef OPEN_EXISTING
    inline constexpr auto OPEN_EXISTING =
#pragma pop_macro("OPEN_EXISTING")
      OPEN_EXISTING;
#pragma push_macro("TRUNCATE_EXISTING")
#undef TRUNCATE_EXISTING
    inline constexpr auto TRUNCATE_EXISTING =
#pragma pop_macro("TRUNCATE_EXISTING")
      TRUNCATE_EXISTING;

#pragma push_macro("FILE_ATTRIBUTE_ARCHIVE")
#undef FILE_ATTRIBUTE_ARCHIVE
    inline constexpr auto FILE_ATTRIBUTE_ARCHIVE =
#pragma pop_macro("FILE_ATTRIBUTE_ARCHIVE")
      FILE_ATTRIBUTE_ARCHIVE;
#pragma push_macro("FILE_ATTRIBUTE_ENCRYPTED")
#undef FILE_ATTRIBUTE_ENCRYPTED
    inline constexpr auto FILE_ATTRIBUTE_ENCRYPTED =
#pragma pop_macro("FILE_ATTRIBUTE_ENCRYPTED")
      FILE_ATTRIBUTE_ENCRYPTED;
#pragma push_macro("FILE_ATTRIBUTE_HIDDEN")
#undef FILE_ATTRIBUTE_HIDDEN
    inline constexpr auto FILE_ATTRIBUTE_HIDDEN =
#pragma pop_macro("FILE_ATTRIBUTE_HIDDEN")
      FILE_ATTRIBUTE_HIDDEN;
#pragma push_macro("FILE_ATTRIBUTE_NORMAL")
#undef FILE_ATTRIBUTE_NORMAL
    inline constexpr auto FILE_ATTRIBUTE_NORMAL =
#pragma pop_macro("FILE_ATTRIBUTE_NORMAL")
      FILE_ATTRIBUTE_NORMAL;
#pragma push_macro("FILE_ATTRIBUTE_OFFLINE")
#undef FILE_ATTRIBUTE_OFFLINE
    inline constexpr auto FILE_ATTRIBUTE_OFFLINE =
#pragma pop_macro("FILE_ATTRIBUTE_OFFLINE")
      FILE_ATTRIBUTE_OFFLINE;
#pragma push_macro("FILE_ATTRIBUTE_READONLY")
#undef FILE_ATTRIBUTE_READONLY
    inline constexpr auto FILE_ATTRIBUTE_READONLY =
#pragma pop_macro("FILE_ATTRIBUTE_READONLY")
      FILE_ATTRIBUTE_READONLY;
#pragma push_macro("FILE_ATTRIBUTE_SYSTEM")
#undef FILE_ATTRIBUTE_SYSTEM
    inline constexpr auto FILE_ATTRIBUTE_SYSTEM =
#pragma pop_macro("FILE_ATTRIBUTE_SYSTEM")
      FILE_ATTRIBUTE_SYSTEM;
#pragma push_macro("FILE_ATTRIBUTE_TEMPORARY")
#undef FILE_ATTRIBUTE_TEMPORARY
    inline constexpr auto FILE_ATTRIBUTE_TEMPORARY =
#pragma pop_macro("FILE_ATTRIBUTE_TEMPORARY")
      FILE_ATTRIBUTE_TEMPORARY;

#pragma push_macro("FILE_FLAG_BACKUP_SEMANTICS")
#undef FILE_FLAG_BACKUP_SEMANTICS
    inline constexpr auto FILE_FLAG_BACKUP_SEMANTICS =
#pragma pop_macro("FILE_FLAG_BACKUP_SEMANTICS")
      FILE_FLAG_BACKUP_SEMANTICS;
#pragma push_macro("FILE_FLAG_DELETE_ON_CLOSE")
#undef FILE_FLAG_DELETE_ON_CLOSE
    inline constexpr auto FILE_FLAG_DELETE_ON_CLOSE =
#pragma pop_macro("FILE_FLAG_DELETE_ON_CLOSE")
      FILE_FLAG_DELETE_ON_CLOSE;
#pragma push_macro("FILE_FLAG_NO_BUFFERING")
#undef FILE_FLAG_NO_BUFFERING
    inline constexpr auto FILE_FLAG_NO_BUFFERING =
#pragma pop_macro("FILE_FLAG_NO_BUFFERING")
      FILE_FLAG_NO_BUFFERING;
#pragma push_macro("FILE_FLAG_OPEN_NO_RECALL")
#undef FILE_FLAG_OPEN_NO_RECALL
    inline constexpr auto FILE_FLAG_OPEN_NO_RECALL =
#pragma pop_macro("FILE_FLAG_OPEN_NO_RECALL")
      FILE_FLAG_OPEN_NO_RECALL;
#pragma push_macro("FILE_FLAG_OPEN_REPARSE_POINT")
#undef FILE_FLAG_OPEN_REPARSE_POINT
    inline constexpr auto FILE_FLAG_OPEN_REPARSE_POINT =
#pragma pop_macro("FILE_FLAG_OPEN_REPARSE_POINT")
      FILE_FLAG_OPEN_REPARSE_POINT;
#pragma push_macro("FILE_FLAG_OVERLAPPED")
#undef FILE_FLAG_OVERLAPPED
    inline constexpr auto FILE_FLAG_OVERLAPPED =
#pragma pop_macro("FILE_FLAG_OVERLAPPED")
      FILE_FLAG_OVERLAPPED;
#pragma push_macro("FILE_FLAG_POSIX_SEMANTICS")
#undef FILE_FLAG_POSIX_SEMANTICS
    inline constexpr auto FILE_FLAG_POSIX_SEMANTICS =
#pragma pop_macro("FILE_FLAG_POSIX_SEMANTICS")
      FILE_FLAG_POSIX_SEMANTICS;
#pragma push_macro("FILE_FLAG_RANDOM_ACCESS")
#undef FILE_FLAG_RANDOM_ACCESS
    inline constexpr auto FILE_FLAG_RANDOM_ACCESS =
#pragma pop_macro("FILE_FLAG_RANDOM_ACCESS")
      FILE_FLAG_RANDOM_ACCESS;
#pragma push_macro("FILE_FLAG_SESSION_AWARE")
#undef FILE_FLAG_SESSION_AWARE
    inline constexpr auto FILE_FLAG_SESSION_AWARE =
#pragma pop_macro("FILE_FLAG_SESSION_AWARE")
      FILE_FLAG_SESSION_AWARE;
#pragma push_macro("FILE_FLAG_SEQUENTIAL_SCAN")
#undef FILE_FLAG_SEQUENTIAL_SCAN
    inline constexpr auto FILE_FLAG_SEQUENTIAL_SCAN =
#pragma pop_macro("FILE_FLAG_SEQUENTIAL_SCAN")
      FILE_FLAG_SEQUENTIAL_SCAN;
#pragma push_macro("FILE_FLAG_WRITE_THROUGH")
#undef FILE_FLAG_WRITE_THROUGH
    inline constexpr auto FILE_FLAG_WRITE_THROUGH =
#pragma pop_macro("FILE_FLAG_WRITE_THROUGH")
      FILE_FLAG_WRITE_THROUGH;

#pragma push_macro("FILE_BEGIN")
#undef FILE_BEGIN
    inline constexpr auto FILE_BEGIN =
#pragma pop_macro("FILE_BEGIN")
      FILE_BEGIN;
#pragma push_macro("FILE_CURRENT")
#undef FILE_CURRENT
    inline constexpr auto FILE_CURRENT =
#pragma pop_macro("FILE_CURRENT")
      FILE_CURRENT;
#pragma push_macro("FILE_END")
#undef FILE_END
    inline constexpr auto FILE_END =
#pragma pop_macro("FILE_END")
      FILE_END;

    // this constant doesn't exists in win32 api, but 0 is a valid value
    inline constexpr auto FILE_SHARE_NOT = 0;
#pragma push_macro("FILE_SHARE_DELETE")
#undef FILE_SHARE_DELETE
    inline constexpr auto FILE_SHARE_DELETE =
#pragma pop_macro("FILE_SHARE_DELETE")
      FILE_SHARE_DELETE;
#pragma push_macro("FILE_SHARE_READ")
#undef FILE_SHARE_READ
    inline constexpr auto FILE_SHARE_READ =
#pragma pop_macro("FILE_SHARE_READ")
      FILE_SHARE_READ;
#pragma push_macro("FILE_SHARE_WRITE")
#undef FILE_SHARE_WRITE
    inline constexpr auto FILE_SHARE_WRITE =
#pragma pop_macro("FILE_SHARE_WRITE")
      FILE_SHARE_WRITE;

#pragma push_macro("FILE_MAP_ALL_ACCESS")
#undef FILE_MAP_ALL_ACCESS
    inline constexpr auto FILE_MAP_ALL_ACCESS =
#pragma pop_macro("FILE_MAP_ALL_ACCESS")
      FILE_MAP_ALL_ACCESS;
#pragma push_macro("FILE_MAP_READ")
#undef FILE_MAP_READ
    inline constexpr auto FILE_MAP_READ =
#pragma pop_macro("FILE_MAP_READ")
      FILE_MAP_READ;
#pragma push_macro("FILE_MAP_WRITE")
#undef FILE_MAP_WRITE
    inline constexpr auto FILE_MAP_WRITE =
#pragma pop_macro("FILE_MAP_WRITE")
      FILE_MAP_WRITE;

#pragma push_macro("FILE_MAP_COPY")
#undef FILE_MAP_COPY
    inline constexpr auto FILE_MAP_COPY =
#pragma pop_macro("FILE_MAP_COPY")
      FILE_MAP_COPY;
#pragma push_macro("FILE_MAP_EXECUTE")
#undef FILE_MAP_EXECUTE
    inline constexpr auto FILE_MAP_EXECUTE =
#pragma pop_macro("FILE_MAP_EXECUTE")
      FILE_MAP_EXECUTE;
#pragma push_macro("FILE_MAP_LARGE_PAGES")
#undef FILE_MAP_LARGE_PAGES
    inline constexpr auto FILE_MAP_LARGE_PAGES =
#pragma pop_macro("FILE_MAP_LARGE_PAGES")
      FILE_MAP_LARGE_PAGES;
#pragma push_macro("FILE_MAP_TARGETS_INVALID")
#undef FILE_MAP_TARGETS_INVALID
    inline constexpr auto FILE_MAP_TARGETS_INVALID =
#pragma pop_macro("FILE_MAP_TARGETS_INVALID")
      FILE_MAP_TARGETS_INVALID;
#pragma push_macro("FILE_MAP_RESERVE")
#undef FILE_MAP_RESERVE
    inline constexpr auto FILE_MAP_RESERVE =
#pragma pop_macro("FILE_MAP_RESERVE")
      FILE_MAP_RESERVE;

#pragma push_macro("EXCEPTION_EXECUTE_HANDLER")
#undef EXCEPTION_EXECUTE_HANDLER
    inline constexpr auto EXCEPTION_EXECUTE_HANDLER =
#pragma pop_macro("EXCEPTION_EXECUTE_HANDLER")
      EXCEPTION_EXECUTE_HANDLER;
#pragma push_macro("EXCEPTION_CONTINUE_SEARCH")
#undef EXCEPTION_CONTINUE_SEARCH
    inline constexpr auto EXCEPTION_CONTINUE_SEARCH =
#pragma pop_macro("EXCEPTION_CONTINUE_SEARCH")
      EXCEPTION_CONTINUE_SEARCH;
#pragma push_macro("EXCEPTION_CONTINUE_EXECUTION")
#undef EXCEPTION_CONTINUE_EXECUTION
    inline constexpr auto EXCEPTION_CONTINUE_EXECUTION =
#pragma pop_macro("EXCEPTION_CONTINUE_EXECUTION")
      EXCEPTION_CONTINUE_EXECUTION;

#pragma push_macro("DPI_AWARENESS_CONTEXT_UNAWARE")
#undef DPI_AWARENESS_CONTEXT_UNAWARE
    inline const /*constexpr*/ auto DPI_AWARENESS_CONTEXT_UNAWARE =
#pragma pop_macro("DPI_AWARENESS_CONTEXT_UNAWARE")
      DPI_AWARENESS_CONTEXT_UNAWARE;
#pragma push_macro("DPI_AWARENESS_CONTEXT_SYSTEM_AWARE")
#undef DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
    inline const /*constexpr*/ auto DPI_AWARENESS_CONTEXT_SYSTEM_AWARE =
#pragma pop_macro("DPI_AWARENESS_CONTEXT_SYSTEM_AWARE")
      DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
#pragma push_macro("DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE")
#undef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
    inline const /*constexpr*/ auto DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE =
#pragma pop_macro("DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE")
      DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
#pragma push_macro("DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2")
#undef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    inline const /*constexpr*/ auto DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 =
#pragma pop_macro("DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2")
      DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
#pragma push_macro("DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED")
#undef DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED
    inline const /*constexpr*/ auto DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED =
#pragma pop_macro("DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED")
      DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED;

#pragma push_macro("CW_USEDEFAULT")
#undef CW_USEDEFAULT
    inline constexpr auto CW_USEDEFAULT =
#pragma pop_macro("CW_USEDEFAULT")
      CW_USEDEFAULT;

#pragma push_macro("WS_BORDER")
#undef WS_BORDER
    inline constexpr auto WS_BORDER =
#pragma pop_macro("WS_BORDER")
      WS_BORDER;
#pragma push_macro("WS_CAPTION")
#undef WS_CAPTION
    inline constexpr auto WS_CAPTION =
#pragma pop_macro("WS_CAPTION")
      WS_CAPTION;
#pragma push_macro("WS_CHILD")
#undef WS_CHILD
    inline constexpr auto WS_CHILD =
#pragma pop_macro("WS_CHILD")
      WS_CHILD;
#pragma push_macro("WS_CHILDWINDOW")
#undef WS_CHILDWINDOW
    inline constexpr auto WS_CHILDWINDOW =
#pragma pop_macro("WS_CHILDWINDOW")
      WS_CHILDWINDOW;
#pragma push_macro("WS_CLIPCHILDREN")
#undef WS_CLIPCHILDREN
    inline constexpr auto WS_CLIPCHILDREN =
#pragma pop_macro("WS_CLIPCHILDREN")
      WS_CLIPCHILDREN;
#pragma push_macro("WS_CLIPSIBLINGS")
#undef WS_CLIPSIBLINGS
    inline constexpr auto WS_CLIPSIBLINGS =
#pragma pop_macro("WS_CLIPSIBLINGS")
      WS_CLIPSIBLINGS;
#pragma push_macro("WS_DISABLED")
#undef WS_DISABLED
    inline constexpr auto WS_DISABLED =
#pragma pop_macro("WS_DISABLED")
      WS_DISABLED;
#pragma push_macro("WS_DLGFRAME")
#undef WS_DLGFRAME
    inline constexpr auto WS_DLGFRAME =
#pragma pop_macro("WS_DLGFRAME")
      WS_DLGFRAME;
#pragma push_macro("WS_GROUP")
#undef WS_GROUP
    inline constexpr auto WS_GROUP =
#pragma pop_macro("WS_GROUP")
      WS_GROUP;
#pragma push_macro("WS_HSCROLL")
#undef WS_HSCROLL
    inline constexpr auto WS_HSCROLL =
#pragma pop_macro("WS_HSCROLL")
      WS_HSCROLL;
#pragma push_macro("WS_ICONIC")
#undef WS_ICONIC
    inline constexpr auto WS_ICONIC =
#pragma pop_macro("WS_ICONIC")
      WS_ICONIC;
#pragma push_macro("WS_MAXIMIZE")
#undef WS_MAXIMIZE
    inline constexpr auto WS_MAXIMIZE =
#pragma pop_macro("WS_MAXIMIZE")
      WS_MAXIMIZE;
#pragma push_macro("WS_MAXIMIZEBOX")
#undef WS_MAXIMIZEBOX
    inline constexpr auto WS_MAXIMIZEBOX =
#pragma pop_macro("WS_MAXIMIZEBOX")
      WS_MAXIMIZEBOX;
#pragma push_macro("WS_MINIMIZE")
#undef WS_MINIMIZE
    inline constexpr auto WS_MINIMIZE =
#pragma pop_macro("WS_MINIMIZE")
      WS_MINIMIZE;
#pragma push_macro("WS_MINIMIZEBOX")
#undef WS_MINIMIZEBOX
    inline constexpr auto WS_MINIMIZEBOX =
#pragma pop_macro("WS_MINIMIZEBOX")
      WS_MINIMIZEBOX;
#pragma push_macro("WS_OVERLAPPED")
#undef WS_OVERLAPPED
    inline constexpr auto WS_OVERLAPPED =
#pragma pop_macro("WS_OVERLAPPED")
      WS_OVERLAPPED;
#pragma push_macro("WS_OVERLAPPEDWINDOW")
#undef WS_OVERLAPPEDWINDOW
    inline constexpr auto WS_OVERLAPPEDWINDOW =
#pragma pop_macro("WS_OVERLAPPEDWINDOW")
      WS_OVERLAPPEDWINDOW;
#pragma push_macro("WS_POPUP")
#undef WS_POPUP
    inline constexpr auto WS_POPUP =
#pragma pop_macro("WS_POPUP")
      WS_POPUP;
#pragma push_macro("WS_POPUPWINDOW")
#undef WS_POPUPWINDOW
    inline constexpr auto WS_POPUPWINDOW =
#pragma pop_macro("WS_POPUPWINDOW")
      WS_POPUPWINDOW;
#pragma push_macro("WS_SIZEBOX")
#undef WS_SIZEBOX
    inline constexpr auto WS_SIZEBOX =
#pragma pop_macro("WS_SIZEBOX")
      WS_SIZEBOX;
#pragma push_macro("WS_SYSMENU")
#undef WS_SYSMENU
    inline constexpr auto WS_SYSMENU =
#pragma pop_macro("WS_SYSMENU")
      WS_SYSMENU;
#pragma push_macro("WS_TABSTOP")
#undef WS_TABSTOP
    inline constexpr auto WS_TABSTOP =
#pragma pop_macro("WS_TABSTOP")
      WS_TABSTOP;
#pragma push_macro("WS_THICKFRAME")
#undef WS_THICKFRAME
    inline constexpr auto WS_THICKFRAME =
#pragma pop_macro("WS_THICKFRAME")
      WS_THICKFRAME;
#pragma push_macro("WS_TILED")
#undef WS_TILED
    inline constexpr auto WS_TILED =
#pragma pop_macro("WS_TILED")
      WS_TILED;
#pragma push_macro("WS_TILEDWINDOW")
#undef WS_TILEDWINDOW
    inline constexpr auto WS_TILEDWINDOW =
#pragma pop_macro("WS_TILEDWINDOW")
      WS_TILEDWINDOW;
#pragma push_macro("WS_VISIBLE")
#undef WS_VISIBLE
    inline constexpr auto WS_VISIBLE =
#pragma pop_macro("WS_VISIBLE")
      WS_VISIBLE;
#pragma push_macro("WS_VSCROLL")
#undef WS_VSCROLL
    inline constexpr auto WS_VSCROLL =
#pragma pop_macro("WS_VSCROLL")
      WS_VSCROLL;

#pragma push_macro("WS_EX_ACCEPTFILES")
#undef WS_EX_ACCEPTFILES
    inline constexpr auto WS_EX_ACCEPTFILES =
#pragma pop_macro("WS_EX_ACCEPTFILES")
      WS_EX_ACCEPTFILES;
#pragma push_macro("WS_EX_APPWINDOW")
#undef WS_EX_APPWINDOW
    inline constexpr auto WS_EX_APPWINDOW =
#pragma pop_macro("WS_EX_APPWINDOW")
      WS_EX_APPWINDOW;
#pragma push_macro("WS_EX_CLIENTEDGE")
#undef WS_EX_CLIENTEDGE
    inline constexpr auto WS_EX_CLIENTEDGE =
#pragma pop_macro("WS_EX_CLIENTEDGE")
      WS_EX_CLIENTEDGE;
#pragma push_macro("WS_EX_COMPOSITED")
#undef WS_EX_COMPOSITED
    inline constexpr auto WS_EX_COMPOSITED =
#pragma pop_macro("WS_EX_COMPOSITED")
      WS_EX_COMPOSITED;
#pragma push_macro("WS_EX_CONTEXTHELP")
#undef WS_EX_CONTEXTHELP
    inline constexpr auto WS_EX_CONTEXTHELP =
#pragma pop_macro("WS_EX_CONTEXTHELP")
      WS_EX_CONTEXTHELP;
#pragma push_macro("WS_EX_CONTROLPARENT")
#undef WS_EX_CONTROLPARENT
    inline constexpr auto WS_EX_CONTROLPARENT =
#pragma pop_macro("WS_EX_CONTROLPARENT")
      WS_EX_CONTROLPARENT;
#pragma push_macro("WS_EX_DLGMODALFRAME")
#undef WS_EX_DLGMODALFRAME
    inline constexpr auto WS_EX_DLGMODALFRAME =
#pragma pop_macro("WS_EX_DLGMODALFRAME")
      WS_EX_DLGMODALFRAME;
#pragma push_macro("WS_EX_LAYERED")
#undef WS_EX_LAYERED
    inline constexpr auto WS_EX_LAYERED =
#pragma pop_macro("WS_EX_LAYERED")
      WS_EX_LAYERED;
#pragma push_macro("WS_EX_LAYOUTRTL")
#undef WS_EX_LAYOUTRTL
    inline constexpr auto WS_EX_LAYOUTRTL =
#pragma pop_macro("WS_EX_LAYOUTRTL")
      WS_EX_LAYOUTRTL;
#pragma push_macro("WS_EX_LEFT")
#undef WS_EX_LEFT
    inline constexpr auto WS_EX_LEFT =
#pragma pop_macro("WS_EX_LEFT")
      WS_EX_LEFT;
#pragma push_macro("WS_EX_LEFTSCROLLBAR")
#undef WS_EX_LEFTSCROLLBAR
    inline constexpr auto WS_EX_LEFTSCROLLBAR =
#pragma pop_macro("WS_EX_LEFTSCROLLBAR")
      WS_EX_LEFTSCROLLBAR;
#pragma push_macro("WS_EX_LTRREADING")
#undef WS_EX_LTRREADING
    inline constexpr auto WS_EX_LTRREADING =
#pragma pop_macro("WS_EX_LTRREADING")
      WS_EX_LTRREADING;
#pragma push_macro("WS_EX_MDICHILD")
#undef WS_EX_MDICHILD
    inline constexpr auto WS_EX_MDICHILD =
#pragma pop_macro("WS_EX_MDICHILD")
      WS_EX_MDICHILD;
#pragma push_macro("WS_EX_NOACTIVATE")
#undef WS_EX_NOACTIVATE
    inline constexpr auto WS_EX_NOACTIVATE =
#pragma pop_macro("WS_EX_NOACTIVATE")
      WS_EX_NOACTIVATE;
#pragma push_macro("WS_EX_NOINHERITLAYOUT")
#undef WS_EX_NOINHERITLAYOUT
    inline constexpr auto WS_EX_NOINHERITLAYOUT =
#pragma pop_macro("WS_EX_NOINHERITLAYOUT")
      WS_EX_NOINHERITLAYOUT;
#pragma push_macro("WS_EX_NOPARENTNOTIFY")
#undef WS_EX_NOPARENTNOTIFY
    inline constexpr auto WS_EX_NOPARENTNOTIFY =
#pragma pop_macro("WS_EX_NOPARENTNOTIFY")
      WS_EX_NOPARENTNOTIFY;
#pragma push_macro("WS_EX_NOREDIRECTIONBITMAP")
#undef WS_EX_NOREDIRECTIONBITMAP
    inline constexpr auto WS_EX_NOREDIRECTIONBITMAP =
#pragma pop_macro("WS_EX_NOREDIRECTIONBITMAP")
      WS_EX_NOREDIRECTIONBITMAP;
#pragma push_macro("WS_EX_OVERLAPPEDWINDOW")
#undef WS_EX_OVERLAPPEDWINDOW
    inline constexpr auto WS_EX_OVERLAPPEDWINDOW =
#pragma pop_macro("WS_EX_OVERLAPPEDWINDOW")
      WS_EX_OVERLAPPEDWINDOW;
#pragma push_macro("WS_EX_PALETTEWINDOW")
#undef WS_EX_PALETTEWINDOW
    inline constexpr auto WS_EX_PALETTEWINDOW =
#pragma pop_macro("WS_EX_PALETTEWINDOW")
      WS_EX_PALETTEWINDOW;
#pragma push_macro("WS_EX_RIGHT")
#undef WS_EX_RIGHT
    inline constexpr auto WS_EX_RIGHT =
#pragma pop_macro("WS_EX_RIGHT")
      WS_EX_RIGHT;
#pragma push_macro("WS_EX_RIGHTSCROLLBAR")
#undef WS_EX_RIGHTSCROLLBAR
    inline constexpr auto WS_EX_RIGHTSCROLLBAR =
#pragma pop_macro("WS_EX_RIGHTSCROLLBAR")
      WS_EX_RIGHTSCROLLBAR;
#pragma push_macro("WS_EX_RTLREADING")
#undef WS_EX_RTLREADING
    inline constexpr auto WS_EX_RTLREADING =
#pragma pop_macro("WS_EX_RTLREADING")
      WS_EX_RTLREADING;
#pragma push_macro("WS_EX_STATICEDGE")
#undef WS_EX_STATICEDGE
    inline constexpr auto WS_EX_STATICEDGE =
#pragma pop_macro("WS_EX_STATICEDGE")
      WS_EX_STATICEDGE;
#pragma push_macro("WS_EX_TOOLWINDOW")
#undef WS_EX_TOOLWINDOW
    inline constexpr auto WS_EX_TOOLWINDOW =
#pragma pop_macro("WS_EX_TOOLWINDOW")
      WS_EX_TOOLWINDOW;
#pragma push_macro("WS_EX_TOPMOST")
#undef WS_EX_TOPMOST
    inline constexpr auto WS_EX_TOPMOST =
#pragma pop_macro("WS_EX_TOPMOST")
      WS_EX_TOPMOST;
#pragma push_macro("WS_EX_TRANSPARENT")
#undef WS_EX_TRANSPARENT
    inline constexpr auto WS_EX_TRANSPARENT =
#pragma pop_macro("WS_EX_TRANSPARENT")
      WS_EX_TRANSPARENT;
#pragma push_macro("WS_EX_WINDOWEDGE")
#undef WS_EX_WINDOWEDGE
    inline constexpr auto WS_EX_WINDOWEDGE =
#pragma pop_macro("WS_EX_WINDOWEDGE")
      WS_EX_WINDOWEDGE;

#pragma push_macro("MONITOR_DEFAULTTONEAREST")
#undef MONITOR_DEFAULTTONEAREST
    inline constexpr auto MONITOR_DEFAULTTONEAREST =
#pragma pop_macro("MONITOR_DEFAULTTONEAREST")
      MONITOR_DEFAULTTONEAREST;
#pragma push_macro("MONITOR_DEFAULTTONULL")
#undef MONITOR_DEFAULTTONULL
    inline constexpr auto MONITOR_DEFAULTTONULL =
#pragma pop_macro("MONITOR_DEFAULTTONULL")
      MONITOR_DEFAULTTONULL;
#pragma push_macro("MONITOR_DEFAULTTOPRIMARY")
#undef MONITOR_DEFAULTTOPRIMARY
    inline constexpr auto MONITOR_DEFAULTTOPRIMARY =
#pragma pop_macro("MONITOR_DEFAULTTOPRIMARY")
      MONITOR_DEFAULTTOPRIMARY;

#pragma push_macro("GWL_STYLE")
#undef GWL_STYLE
    inline constexpr auto GWL_STYLE
#pragma pop_macro("GWL_STYLE")
      = GWL_STYLE;
#pragma push_macro("GWL_HINSTANCE")
#ifndef _WIN64
    #undef GWL_HINSTANCE
    inline constexpr auto GWL_HINSTANCE
    #pragma pop_macro("GWL_HINSTANCE")
      = GWL_HINSTANCE;
    #pragma push_macro("GWL_HWNDPARENT")
    #undef GWL_HWNDPARENT
    inline constexpr auto GWL_HWNDPARENT
    #pragma pop_macro("GWL_HWNDPARENT")
      = GWL_HWNDPARENT;
    #pragma push_macro("GWL_USERDATA")
    #undef GWL_USERDATA
    inline constexpr auto GWL_USERDATA
    #pragma pop_macro("GWL_USERDATA")
      = GWL_USERDATA;
    #pragma push_macro("GWL_WNDPROC")
    #undef GWL_WNDPROC
    inline constexpr auto GWL_WNDPROC
    #pragma pop_macro("GWL_WNDPROC")
      = GWL_WNDPROC;
#endif
#pragma push_macro("GWL_ID")
#undef GWL_ID
    inline constexpr auto GWL_ID
#pragma pop_macro("GWL_ID")
      = GWL_ID;
#pragma push_macro("GWL_EXSTYLE")
#undef GWL_EXSTYLE
    inline constexpr auto GWL_EXSTYLE
#pragma pop_macro("GWL_EXSTYLE")
      = GWL_EXSTYLE;

#ifndef _WIN64
    #pragma push_macro("DWL_DLGPROC")
    #undef DWL_DLGPROC
    inline constexpr auto DWL_DLGPROC
    #pragma pop_macro("DWL_DLGPROC")
      = DWL_DLGPROC;
    #pragma push_macro("DWL_MSGRESULT")
    #undef DWL_MSGRESULT
    inline constexpr auto DWL_MSGRESULT
    #pragma pop_macro("DWL_MSGRESULT")
      = DWL_MSGRESULT;
    #pragma push_macro("DWL_USER")
    #undef DWL_USER
    inline constexpr auto DWL_USER
    #pragma pop_macro("DWL_USER")
      = DWL_USER;
#endif

#pragma push_macro("PM_NOREMOVE")
#undef PM_NOREMOVE
    inline constexpr auto PM_NOREMOVE
#pragma pop_macro("PM_NOREMOVE")
      = PM_NOREMOVE;
#pragma push_macro("PM_REMOVE")
#undef PM_REMOVE
    inline constexpr auto PM_REMOVE
#pragma pop_macro("PM_REMOVE")
      = PM_REMOVE;
#pragma push_macro("PM_NOYIELD")
#undef PM_NOYIELD
    inline constexpr auto PM_NOYIELD
#pragma pop_macro("PM_NOYIELD")
      = PM_NOYIELD;

#pragma push_macro("PM_QS_INPUT")
#undef PM_QS_INPUT
    inline constexpr auto PM_QS_INPUT
#pragma pop_macro("PM_QS_INPUT")
      = PM_QS_INPUT;
#pragma push_macro("PM_QS_POSTMESSAGE")
#undef PM_QS_POSTMESSAGE
    inline constexpr auto PM_QS_POSTMESSAGE
#pragma pop_macro("PM_QS_POSTMESSAGE")
      = PM_QS_POSTMESSAGE;
#pragma push_macro("PM_QS_PAINT")
#undef PM_QS_PAINT
    inline constexpr auto PM_QS_PAINT
#pragma pop_macro("PM_QS_PAINT")
      = PM_QS_PAINT;
#pragma push_macro("PM_QS_SENDMESSAGE")
#undef PM_QS_SENDMESSAGE
    inline constexpr auto PM_QS_SENDMESSAGE
#pragma pop_macro("PM_QS_SENDMESSAGE")
      = PM_QS_SENDMESSAGE;

#pragma push_macro("SW_HIDE")
#undef SW_HIDE
    inline constexpr auto SW_HIDE
#pragma pop_macro("SW_HIDE")
      = SW_HIDE;
#pragma push_macro("SW_SHOWNORMAL")
#undef SW_SHOWNORMAL
    inline constexpr auto SW_SHOWNORMAL
#pragma pop_macro("SW_SHOWNORMAL")
      = SW_SHOWNORMAL;
#pragma push_macro("SW_NORMAL")
#undef SW_NORMAL
    inline constexpr auto SW_NORMAL
#pragma pop_macro("SW_NORMAL")
      = SW_NORMAL;
#pragma push_macro("SW_SHOWMINIMIZED")
#undef SW_SHOWMINIMIZED
    inline constexpr auto SW_SHOWMINIMIZED
#pragma pop_macro("SW_SHOWMINIMIZED")
      = SW_SHOWMINIMIZED;
#pragma push_macro("SW_SHOWMAXIMIZED")
#undef SW_SHOWMAXIMIZED
    inline constexpr auto SW_SHOWMAXIMIZED
#pragma pop_macro("SW_SHOWMAXIMIZED")
      = SW_SHOWMAXIMIZED;
#pragma push_macro("SW_MAXIMIZE")
#undef SW_MAXIMIZE
    inline constexpr auto SW_MAXIMIZE
#pragma pop_macro("SW_MAXIMIZE")
      = SW_MAXIMIZE;
#pragma push_macro("SW_SHOWNOACTIVATE")
#undef SW_SHOWNOACTIVATE
    inline constexpr auto SW_SHOWNOACTIVATE
#pragma pop_macro("SW_SHOWNOACTIVATE")
      = SW_SHOWNOACTIVATE;
#pragma push_macro("SW_SHOW")
#undef SW_SHOW
    inline constexpr auto SW_SHOW
#pragma pop_macro("SW_SHOW")
      = SW_SHOW;
#pragma push_macro("SW_MINIMIZE")
#undef SW_MINIMIZE
    inline constexpr auto SW_MINIMIZE
#pragma pop_macro("SW_MINIMIZE")
      = SW_MINIMIZE;
#pragma push_macro("SW_SHOWMINNOACTIVE")
#undef SW_SHOWMINNOACTIVE
    inline constexpr auto SW_SHOWMINNOACTIVE
#pragma pop_macro("SW_SHOWMINNOACTIVE")
      = SW_SHOWMINNOACTIVE;
#pragma push_macro("SW_SHOWNA")
#undef SW_SHOWNA
    inline constexpr auto SW_SHOWNA
#pragma pop_macro("SW_SHOWNA")
      = SW_SHOWNA;
#pragma push_macro("SW_RESTORE")
#undef SW_RESTORE
    inline constexpr auto SW_RESTORE
#pragma pop_macro("SW_RESTORE")
      = SW_RESTORE;
#pragma push_macro("SW_SHOWDEFAULT")
#undef SW_SHOWDEFAULT
    inline constexpr auto SW_SHOWDEFAULT
#pragma pop_macro("SW_SHOWDEFAULT")
      = SW_SHOWDEFAULT;
#pragma push_macro("SW_FORCEMINIMIZE")
#undef SW_FORCEMINIMIZE
    inline constexpr auto SW_FORCEMINIMIZE
#pragma pop_macro("SW_FORCEMINIMIZE")
      = SW_FORCEMINIMIZE;

#pragma push_macro("WM_CAPTURECHANGED")
#undef WM_CAPTURECHANGED
    inline constexpr auto WM_CAPTURECHANGED
#pragma pop_macro("WM_CAPTURECHANGED")
      = WM_CAPTURECHANGED;
#pragma push_macro("WM_LBUTTONDBLCLK")
#undef WM_LBUTTONDBLCLK
    inline constexpr auto WM_LBUTTONDBLCLK
#pragma pop_macro("WM_LBUTTONDBLCLK")
      = WM_LBUTTONDBLCLK;
#pragma push_macro("WM_LBUTTONDOWN")
#undef WM_LBUTTONDOWN
    inline constexpr auto WM_LBUTTONDOWN
#pragma pop_macro("WM_LBUTTONDOWN")
      = WM_LBUTTONDOWN;
#pragma push_macro("WM_LBUTTONUP")
#undef WM_LBUTTONUP
    inline constexpr auto WM_LBUTTONUP
#pragma pop_macro("WM_LBUTTONUP")
      = WM_LBUTTONUP;
#pragma push_macro("WM_MBUTTONDBLCLK")
#undef WM_MBUTTONDBLCLK
    inline constexpr auto WM_MBUTTONDBLCLK
#pragma pop_macro("WM_MBUTTONDBLCLK")
      = WM_MBUTTONDBLCLK;
#pragma push_macro("WM_MBUTTONDOWN")
#undef WM_MBUTTONDOWN
    inline constexpr auto WM_MBUTTONDOWN
#pragma pop_macro("WM_MBUTTONDOWN")
      = WM_MBUTTONDOWN;
#pragma push_macro("WM_MBUTTONUP")
#undef WM_MBUTTONUP
    inline constexpr auto WM_MBUTTONUP
#pragma pop_macro("WM_MBUTTONUP")
      = WM_MBUTTONUP;
#pragma push_macro("WM_MOUSEACTIVATE")
#undef WM_MOUSEACTIVATE
    inline constexpr auto WM_MOUSEACTIVATE
#pragma pop_macro("WM_MOUSEACTIVATE")
      = WM_MOUSEACTIVATE;
#pragma push_macro("WM_MOUSEHOVER")
#undef WM_MOUSEHOVER
    inline constexpr auto WM_MOUSEHOVER
#pragma pop_macro("WM_MOUSEHOVER")
      = WM_MOUSEHOVER;
#pragma push_macro("WM_MOUSEHWHEEL")
#undef WM_MOUSEHWHEEL
    inline constexpr auto WM_MOUSEHWHEEL
#pragma pop_macro("WM_MOUSEHWHEEL")
      = WM_MOUSEHWHEEL;
#pragma push_macro("WM_MOUSELEAVE")
#undef WM_MOUSELEAVE
    inline constexpr auto WM_MOUSELEAVE
#pragma pop_macro("WM_MOUSELEAVE")
      = WM_MOUSELEAVE;
#pragma push_macro("WM_MOUSEMOVE")
#undef WM_MOUSEMOVE
    inline constexpr auto WM_MOUSEMOVE
#pragma pop_macro("WM_MOUSEMOVE")
      = WM_MOUSEMOVE;
#pragma push_macro("WM_MOUSEWHEEL")
#undef WM_MOUSEWHEEL
    inline constexpr auto WM_MOUSEWHEEL
#pragma pop_macro("WM_MOUSEWHEEL")
      = WM_MOUSEWHEEL;
#pragma push_macro("WM_NCHITTEST")
#undef WM_NCHITTEST
    inline constexpr auto WM_NCHITTEST
#pragma pop_macro("WM_NCHITTEST")
      = WM_NCHITTEST;
#pragma push_macro("WM_NCLBUTTONDBLCLK")
#undef WM_NCLBUTTONDBLCLK
    inline constexpr auto WM_NCLBUTTONDBLCLK
#pragma pop_macro("WM_NCLBUTTONDBLCLK")
      = WM_NCLBUTTONDBLCLK;
#pragma push_macro("WM_NCLBUTTONDOWN")
#undef WM_NCLBUTTONDOWN
    inline constexpr auto WM_NCLBUTTONDOWN
#pragma pop_macro("WM_NCLBUTTONDOWN")
      = WM_NCLBUTTONDOWN;
#pragma push_macro("WM_NCLBUTTONUP")
#undef WM_NCLBUTTONUP
    inline constexpr auto WM_NCLBUTTONUP
#pragma pop_macro("WM_NCLBUTTONUP")
      = WM_NCLBUTTONUP;
#pragma push_macro("WM_NCMBUTTONDBLCLK")
#undef WM_NCMBUTTONDBLCLK
    inline constexpr auto WM_NCMBUTTONDBLCLK
#pragma pop_macro("WM_NCMBUTTONDBLCLK")
      = WM_NCMBUTTONDBLCLK;
#pragma push_macro("WM_NCMBUTTONDOWN")
#undef WM_NCMBUTTONDOWN
    inline constexpr auto WM_NCMBUTTONDOWN
#pragma pop_macro("WM_NCMBUTTONDOWN")
      = WM_NCMBUTTONDOWN;
#pragma push_macro("WM_NCMBUTTONUP")
#undef WM_NCMBUTTONUP
    inline constexpr auto WM_NCMBUTTONUP
#pragma pop_macro("WM_NCMBUTTONUP")
      = WM_NCMBUTTONUP;
#pragma push_macro("WM_NCMOUSEHOVER")
#undef WM_NCMOUSEHOVER
    inline constexpr auto WM_NCMOUSEHOVER
#pragma pop_macro("WM_NCMOUSEHOVER")
      = WM_NCMOUSEHOVER;
#pragma push_macro("WM_NCMOUSELEAVE")
#undef WM_NCMOUSELEAVE
    inline constexpr auto WM_NCMOUSELEAVE
#pragma pop_macro("WM_NCMOUSELEAVE")
      = WM_NCMOUSELEAVE;
#pragma push_macro("WM_NCMOUSEMOVE")
#undef WM_NCMOUSEMOVE
    inline constexpr auto WM_NCMOUSEMOVE
#pragma pop_macro("WM_NCMOUSEMOVE")
      = WM_NCMOUSEMOVE;
#pragma push_macro("WM_NCRBUTTONDBLCLK")
#undef WM_NCRBUTTONDBLCLK
    inline constexpr auto WM_NCRBUTTONDBLCLK
#pragma pop_macro("WM_NCRBUTTONDBLCLK")
      = WM_NCRBUTTONDBLCLK;
#pragma push_macro("WM_NCRBUTTONDOWN")
#undef WM_NCRBUTTONDOWN
    inline constexpr auto WM_NCRBUTTONDOWN
#pragma pop_macro("WM_NCRBUTTONDOWN")
      = WM_NCRBUTTONDOWN;
#pragma push_macro("WM_NCRBUTTONUP")
#undef WM_NCRBUTTONUP
    inline constexpr auto WM_NCRBUTTONUP
#pragma pop_macro("WM_NCRBUTTONUP")
      = WM_NCRBUTTONUP;
#pragma push_macro("WM_NCXBUTTONDBLCLK")
#undef WM_NCXBUTTONDBLCLK
    inline constexpr auto WM_NCXBUTTONDBLCLK
#pragma pop_macro("WM_NCXBUTTONDBLCLK")
      = WM_NCXBUTTONDBLCLK;
#pragma push_macro("WM_NCXBUTTONDOWN")
#undef WM_NCXBUTTONDOWN
    inline constexpr auto WM_NCXBUTTONDOWN
#pragma pop_macro("WM_NCXBUTTONDOWN")
      = WM_NCXBUTTONDOWN;
#pragma push_macro("WM_NCXBUTTONUP")
#undef WM_NCXBUTTONUP
    inline constexpr auto WM_NCXBUTTONUP
#pragma pop_macro("WM_NCXBUTTONUP")
      = WM_NCXBUTTONUP;
#pragma push_macro("WM_RBUTTONDBLCLK")
#undef WM_RBUTTONDBLCLK
    inline constexpr auto WM_RBUTTONDBLCLK
#pragma pop_macro("WM_RBUTTONDBLCLK")
      = WM_RBUTTONDBLCLK;
#pragma push_macro("WM_RBUTTONDOWN")
#undef WM_RBUTTONDOWN
    inline constexpr auto WM_RBUTTONDOWN
#pragma pop_macro("WM_RBUTTONDOWN")
      = WM_RBUTTONDOWN;
#pragma push_macro("WM_RBUTTONUP")
#undef WM_RBUTTONUP
    inline constexpr auto WM_RBUTTONUP
#pragma pop_macro("WM_RBUTTONUP")
      = WM_RBUTTONUP;
#pragma push_macro("WM_XBUTTONDBLCLK")
#undef WM_XBUTTONDBLCLK
    inline constexpr auto WM_XBUTTONDBLCLK
#pragma pop_macro("WM_XBUTTONDBLCLK")
      = WM_XBUTTONDBLCLK;
#pragma push_macro("WM_XBUTTONDOWN")
#undef WM_XBUTTONDOWN
    inline constexpr auto WM_XBUTTONDOWN
#pragma pop_macro("WM_XBUTTONDOWN")
      = WM_XBUTTONDOWN;
#pragma push_macro("WM_XBUTTONUP")
#undef WM_XBUTTONUP
    inline constexpr auto WM_XBUTTONUP
#pragma pop_macro("WM_XBUTTONUP")
      = WM_XBUTTONUP;

#pragma push_macro("MK_CONTROL")
#undef MK_CONTROL
    inline constexpr auto MK_CONTROL
#pragma pop_macro("MK_CONTROL")
      = MK_CONTROL;
#pragma push_macro("MK_LBUTTON")
#undef MK_LBUTTON
    inline constexpr auto MK_LBUTTON
#pragma pop_macro("MK_LBUTTON")
      = MK_LBUTTON;
#pragma push_macro("MK_MBUTTON")
#undef MK_MBUTTON
    inline constexpr auto MK_MBUTTON
#pragma pop_macro("MK_MBUTTON")
      = MK_MBUTTON;
#pragma push_macro("MK_RBUTTON")
#undef MK_RBUTTON
    inline constexpr auto MK_RBUTTON
#pragma pop_macro("MK_RBUTTON")
      = MK_RBUTTON;
#pragma push_macro("MK_SHIFT")
#undef MK_SHIFT
    inline constexpr auto MK_SHIFT
#pragma pop_macro("MK_SHIFT")
      = MK_SHIFT;
#pragma push_macro("MK_XBUTTON1")
#undef MK_XBUTTON1
    inline constexpr auto MK_XBUTTON1
#pragma pop_macro("MK_XBUTTON1")
      = MK_XBUTTON1;
#pragma push_macro("MK_XBUTTON2")
#undef MK_XBUTTON2
    inline constexpr auto MK_XBUTTON2
#pragma pop_macro("MK_XBUTTON2")
      = MK_XBUTTON2;

#pragma push_macro("XBUTTON1")
#undef XBUTTON1
    inline constexpr auto XBUTTON1
#pragma pop_macro("XBUTTON1")
      = XBUTTON1;
#pragma push_macro("XBUTTON2")
#undef XBUTTON2
    inline constexpr auto XBUTTON2
#pragma pop_macro("XBUTTON2")
      = XBUTTON2;
} // namespace win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto MakeHRESULT(LONG severity, LONG facility, LONG code) noexcept -> HRESULT {
        return MAKE_HRESULT(severity, facility, code);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Code(HRESULT hr) noexcept -> bool {
        return HRESULT_CODE(hr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Severity(HRESULT hr) noexcept -> bool {
        return HRESULT_SEVERITY(hr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Facility(HRESULT hr) noexcept -> bool {
        return HRESULT_FACILITY(hr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Succeeded(HRESULT hr) noexcept -> bool {
        return SUCCEEDED(hr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Failed(HRESULT hr) noexcept -> bool {
        return FAILED(hr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetXLPARAM(LPARAM lp) noexcept -> int {
        return GET_X_LPARAM(lp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetYLPARAM(LPARAM lp) noexcept -> int {
        return GET_Y_LPARAM(lp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetAppCommandLPARAM(LPARAM wp) noexcept -> int {
        return GET_APPCOMMAND_LPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetDeviceLPARAM(LPARAM wp) noexcept -> int {
        return GET_DEVICE_LPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetFlagsLPARAM(LPARAM wp) noexcept -> int {
        return GET_FLAGS_LPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetKeyStateLPARAM(LPARAM wp) noexcept -> int {
        return GET_KEYSTATE_LPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetKeyStateWPARAM(WPARAM wp) noexcept -> int {
        return GET_KEYSTATE_WPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetRawInputCodeWPARAM(WPARAM wp) noexcept -> int {
        return GET_RAWINPUT_CODE_WPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetNCHittestWPARAM(WPARAM wp) noexcept -> int {
        return GET_NCHITTEST_WPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetWheelDeltaWPARAM(WPARAM wp) noexcept -> int {
        return GET_WHEEL_DELTA_WPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto GetXButtonWPARAM(WPARAM wp) noexcept -> int {
        return GET_XBUTTON_WPARAM(wp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Rgb(BYTE r, BYTE g, BYTE b) noexcept -> COLORREF {
        return RGB(r, g, b);
    }
} // namespace win32
