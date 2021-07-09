// Codepage Conversion Library for Process FileIO
#include <windows.h>

#define EXPORTABLE __declspec(dllexport)
EXPORTABLE void dropkick(){}

// Default CodePage
#define CP_UTF8 65001
static unsigned int target_codepage;

void init_target_codepage(){
	if(!getenv("LOCALE_CP")){target_codepage = CP_UTF8; return;}
	target_codepage = atoi(getenv("LOCALE_CP"));
}

typedef HANDLE __stdcall tCreateFileA(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
typedef HANDLE __stdcall tFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
typedef BOOL   __stdcall tSetCurrentDirectoryA(LPCSTR lpPathName);

static tSetCurrentDirectoryA* real_SetCurrentDirectoryA = NULL;
static tFindFirstFileA* real_FindFirstFileA = NULL;
static tCreateFileA* real_CreateFileA = NULL;

// For a given string, evaluate if it's printable ascii.
#define PRINTABLE_ASCII_MAX 0x7E
#define PRINTABLE_ASCII_MIN 0x20
unsigned char is_printable_ascii(const char* instr){
    int i = 0;
    for(i=0;i<strlen(instr);i++){
        if(instr[i] > PRINTABLE_ASCII_MAX || instr[i] < PRINTABLE_ASCII_MIN){ return FALSE;}
    }
    return TRUE;
}

BOOL __stdcall x_SetCurrentDirectoryA(LPCSTR lpPathName){
    if(lpPathName && !is_printable_ascii(lpPathName)){
        wchar_t w_file_path[1024] = {0x00};
        if(MultiByteToWideChar(target_codepage, 0, lpPathName, -1, w_file_path, sizeof(w_file_path))){
            return SetCurrentDirectoryW(w_file_path);
        }
    }
    // Bypass Conversion
    return real_SetCurrentDirectoryA(lpPathName);
}

HANDLE __stdcall x_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData){
    if(lpFileName && !is_printable_ascii(lpFileName)){
        wchar_t w_file_path[1024] = {0x00};
        if(MultiByteToWideChar(target_codepage, 0, lpFileName, -1, w_file_path, sizeof(w_file_path))){
            LPWIN32_FIND_DATAW lpFindFileDataW = NULL;
            HANDLE hFile = FindFirstFileW(w_file_path,lpFindFileDataW);
            lpFindFileData->dwFileAttributes = lpFindFileDataW->dwFileAttributes;
            lpFindFileData->ftCreationTime   = lpFindFileDataW->ftCreationTime;
            lpFindFileData->ftLastAccessTime = lpFindFileDataW->ftLastAccessTime;
            lpFindFileData->ftLastWriteTime  = lpFindFileDataW->ftLastWriteTime;
            lpFindFileData->nFileSizeHigh    = lpFindFileDataW->nFileSizeHigh;
            lpFindFileData->nFileSizeLow     = lpFindFileDataW->nFileSizeLow;
            lpFindFileData->dwReserved0      = lpFindFileDataW->dwReserved0;
            lpFindFileData->dwReserved1      = lpFindFileDataW->dwReserved1;
            WideCharToMultiByte(target_codepage, 0, lpFindFileDataW->cFileName, -1,
                                lpFindFileData->cFileName, sizeof(lpFindFileData->cFileName),
                                NULL, NULL);
            WideCharToMultiByte(target_codepage, 0, lpFindFileDataW->cAlternateFileName, -1,
                                lpFindFileData->cAlternateFileName, sizeof(lpFindFileData->cAlternateFileName),
                                NULL, NULL);
            return hFile;
        }
    }
    return real_FindFirstFileA(lpFileName, lpFindFileData);
}

HANDLE __stdcall x_CreateFileA(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile){
    if(lpFileName && !is_printable_ascii(lpFileName)){
        wchar_t w_file_path[1024] = {0x00};
        if(MultiByteToWideChar(target_codepage, 0, lpFileName, -1, w_file_path, sizeof(w_file_path))){
            return CreateFileW(w_file_path,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
        }
    }
    return real_CreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

// Entry-Point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        init_target_codepage();
		// ADD HOOKS HERE
    }
    return TRUE;
}
