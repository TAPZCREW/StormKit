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
#include <shlwapi.h>
#include <taskschd.h>
#include <threadpoolapiset.h>
#include <wincodec.h>
#include <wincrypt.h>
#include <windowsx.h>
#include <winnt.h>
#include <winsock2.h>
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
    using ::FlushFileBuffers;
    using ::FormatMessageA;
    using ::FormatMessageW;
    using ::FreeLibrary;
    using ::FreeSid;
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
    using ::SetThreadDescription;
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
    using ::WPARAM;
    using ::WriteFile;

    auto get_x_lparam(LPARAM) noexcept -> int;
    auto get_y_lparam(LPARAM) noexcept -> int;

#undef INVALID_HANDLE_VALUE
    inline const /*constexpr*/ auto INVALID_HANDLE_VALUE = ((HANDLE)(ULONG_PTR)-1);

#undef PAGE_EXECUTE
    inline constexpr auto PAGE_EXECUTE = 0x10;
#undef PAGE_EXECUTE_READ
    inline constexpr auto PAGE_EXECUTE_READ = 0x20;
#undef PAGE_EXECUTE_READWRITE
    inline constexpr auto PAGE_EXECUTE_READWRITE = 0x40;
#undef PAGE_EXECUTE_WRITECOPY
    inline constexpr auto PAGE_EXECUTE_WRITECOPY = 0x80;
#undef PAGE_NOACCESS
    inline constexpr auto PAGE_NOACCESS = 0x01;
#undef PAGE_READONLY
    inline constexpr auto PAGE_READONLY = 0x02;
#undef PAGE_READWRITE
    inline constexpr auto PAGE_READWRITE = 0x04;
#undef PAGE_WRITECOPY
    inline constexpr auto PAGE_WRITECOPY = 0x08;
#undef PAGE_TARGETS_INVALID
    inline constexpr auto PAGE_TARGETS_INVALID = 0x40000000;
#undef PAGE_TARGETS_NO_UPDATE
    inline constexpr auto PAGE_TARGETS_NO_UPDATE = 0x40000000;

#undef PAGE_GUARD
    inline constexpr auto PAGE_GUARD = 0x100;
#undef PAGE_NOCACHE
    inline constexpr auto PAGE_NOCACHE = 0x200;
#undef PAGE_WRITECOMBINE
    inline constexpr auto PAGE_WRITECOMBINE = 0x400;

#undef GENERIC_ALL
    inline constexpr auto GENERIC_ALL = 0x10000000;
#undef GENERIC_EXECUTE
    inline constexpr auto GENERIC_EXECUTE = 0x20000000;
#undef GENERIC_WRITE
    inline constexpr auto GENERIC_WRITE = 0x40000000;
#undef GENERIC_READ
    inline constexpr auto GENERIC_READ = 0x80000000;

#undef CREATE_ALWAYS
    inline constexpr auto CREATE_ALWAYS = 2;
#undef CREATE_NEW
    inline constexpr auto CREATE_NEW = 1;
#undef OPEN_ALWAYS
    inline constexpr auto OPEN_ALWAYS = 4;
#undef OPEN_EXISTING
    inline constexpr auto OPEN_EXISTING = 3;
#undef TRUNCATE_EXISTING
    inline constexpr auto TRUNCATE_EXISTING = 5;

#undef FILE_BEGIN
    inline constexpr auto FILE_BEGIN = 0;
#undef FILE_CURRENT
    inline constexpr auto FILE_CURRENT = 1;
#undef FILE_END
    inline constexpr auto FILE_END = 2;

    // this constant doesn't exists in win32 api, but 0 is a valid value
    inline constexpr auto FILE_SHARE_NOT = 0;
#undef FILE_SHARE_DELETE
    inline constexpr auto FILE_SHARE_DELETE = 4;
#undef FILE_SHARE_READ
    inline constexpr auto FILE_SHARE_READ = 1;
#undef FILE_SHARE_WRITE
    inline constexpr auto FILE_SHARE_WRITE = 2;

#undef FILE_MAP_ALL_ACCESS
    inline constexpr auto FILE_MAP_ALL_ACCESS = SECTION_ALL_ACCESS;
#undef FILE_MAP_READ
    inline constexpr auto FILE_MAP_READ = SECTION_MAP_READ;
#undef FILE_MAP_WRITE
    inline constexpr auto FILE_MAP_WRITE = SECTION_MAP_WRITE;

#undef FILE_MAP_COPY
    inline constexpr auto FILE_MAP_COPY = 0x1;
#undef FILE_MAP_EXECUTE
    inline constexpr auto FILE_MAP_EXECUTE = SECTION_MAP_EXECUTE_EXPLICIT;
#undef FILE_MAP_LARGE_PAGES
    inline constexpr auto FILE_MAP_LARGE_PAGES = 0x20000000;
#undef FILE_MAP_TARGETS_INVALID
    inline constexpr auto FILE_MAP_TARGETS_INVALID = 0x40000000;
#undef FILE_MAP_RESERVE
    inline constexpr auto FILE_MAP_RESERVE = 0x80000000;

#undef EXCEPTION_EXECUTE_HANDLER
    inline constexpr auto EXCEPTION_EXECUTE_HANDLER = 1;
#undef EXCEPTION_CONTINUE_SEARCH
    inline constexpr auto EXCEPTION_CONTINUE_SEARCH = 0;
#undef EXCEPTION_CONTINUE_EXECUTION
    inline constexpr auto EXCEPTION_CONTINUE_EXECUTION = -1;
} // namespace win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto get_x_lparam(LPARAM lp) noexcept -> int {
        return GET_X_LPARAM(lp);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto get_y_lparam(LPARAM lp) noexcept -> int {
        return GET_Y_LPARAM(lp);
    }
} // namespace win32
