//#undef UNICODE
#define _NO_CRT_STDIO_INLINE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>

LRESULT  list_additem(WCHAR *sys_name, WCHAR *operation, WCHAR *status, WCHAR *last_error);

WCHAR g_error_code[1024] = {0}; //错误码


void get_last_error()
{
    unsigned int nRet;    // kr00_4
    DWORD ecode;        // [esp-28h] [ebp-28h]
    nRet = __getcallerseflags();
    ecode = GetLastError();
    RtlZeroMemory(g_error_code, 256);
    if (!FormatMessage(0x10FFu, 0, ecode, 0x400u, g_error_code, 0x80u, 0))
    {
        lstrcpy(g_error_code, L"Error number not found.");
    }

    __writeeflags(nRet);

}


BOOL RegSetVal(HKEY hRoot, LPCWSTR szSubkey, unsigned long nType, LPCWSTR szValueName, LPCWSTR szValue)
{
	HKEY hKey = NULL;
	long lRet = 0;
	lRet = RegCreateKeyEx(hRoot, szSubkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (lRet != ERROR_SUCCESS)
	{
		return FALSE;
	}
	lRet = RegSetValueEx(hKey, szValueName, 0, nType, (const BYTE*)szValue, wcslen(szValue)*2);
	if (lRet != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegCloseKey(hKey);
	return TRUE;
}




int  sys_load(WCHAR *lpServiceName, WCHAR *lpBinaryPathName)
{
    int st=0;           // ebx
    WCHAR *status = 0;    // edi
    WCHAR szSvcHost[1024]={0};
    SC_HANDLE hScm = 0; // eax
    SC_HANDLE hService  = 0; // eax
    status = L"失败";
    hScm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if (hScm)
    {

        WCHAR *exName=_wcslwr(&lpBinaryPathName[wcslen(lpBinaryPathName)-4]);
         if (lstrcmpi(L".sys", exName)==0)
         {
            hService =CreateService(hScm, lpServiceName, lpServiceName, 0, 1u, 3u, 0, lpBinaryPathName, 0, 0, 0, 0, 0);
            //设置过滤器驱动注册表

         }else if (lstrcmpi(L".exe", exName)==0)
         {
            //服务exe
            hService = CreateService(hScm, lpServiceName, lpServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);

         }else if (lstrcmpi(L".dll", exName)==0)
         {
            //服务dll
            wchar_t servicsdll[260]={0};
            RegSetVal(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", REG_MULTI_SZ, lpServiceName, lpServiceName);
            wsprintf(szSvcHost,L"%%SystemRoot%%\\System32\\svchost.exe -k %s",lpServiceName);
            hService = CreateService(hScm, lpServiceName, lpServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szSvcHost, NULL, NULL, NULL, NULL, NULL);
            wsprintf(servicsdll,L"SYSTEM\\CurrentControlSet\\services\\%s\\Parameters",lpServiceName);
            RegSetVal(HKEY_LOCAL_MACHINE, servicsdll, REG_EXPAND_SZ, L"ServiceDll", lpBinaryPathName);
            
         }
         
        get_last_error();
        if (hService)
        {
            CloseServiceHandle(hService);
            st = 1;
            status = L"完成";
        }
        CloseServiceHandle(hScm);
    }
    else
    {
        MessageBox(0, L"无法获取服务控制管理器句柄", L"错误", 0x10u);
    }

    list_additem(lpServiceName, L"注册", status, g_error_code);
    return st;
}



int  sys_unload(WCHAR *lpServiceName)
{
    int st = 0;       // ebx
    WCHAR *status = 0;    // edi
    SC_HANDLE hScm = 0; // eax
    SC_HANDLE hService = 0; // eax
    status = L"失败";
    hScm = OpenSCManager(0, 0, 1u);
    if (hScm)
    {

        hService=OpenService(hScm, lpServiceName, 0x10000u);
        get_last_error();
        if (hService)
        {
           
            BOOL ds=DeleteService(hService);
            get_last_error();
            if (ds)
            {
                st = 1;
                status = L"完成";
            }
            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hScm);
    }
    else
    {
        MessageBox(0, L"无法获取服务控制管理器句柄", L"错误", 0x10u);
    }
    list_additem(lpServiceName, L"卸载", status, g_error_code);
    return st;
}


int  sys_start(WCHAR *lpServiceName)
{
    int st = 0;             // ebx
    WCHAR *status = 0;      // edi
    SC_HANDLE hScm = 0;     // eax
    SC_HANDLE hService = 0; // eax
    status = L"失败";

    hScm = OpenSCManager(0, 0, 1u);
    if (hScm)
    {

        hService = OpenService(hScm, lpServiceName, 0x10u);
        get_last_error();
        if (hService)
        {

            BOOL ss = StartService(hService, 0, 0);
            get_last_error();
            if (ss)
            {
                st = 1;
                status = L"完成";
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hScm);
    }
    else
    {
        MessageBox(0, L"无法获取服务控制管理器句柄", L"错误", 0x10u);
    }
    list_additem(lpServiceName, L"启动", status, g_error_code);
    return st;
}


int  sys_stop(WCHAR *lpServiceName)
{
    int st = 0;                           // ebx
    WCHAR *status = 0;                    // edi
    SC_HANDLE hScm = 0;                   // eax
    SC_HANDLE hService = 0;               // eax
    int v6 = 0;                           // eax
    struct _SERVICE_STATUS ServiceStatus; // [esp+8h] [ebp-1Ch] BYREF

    status = L"失败";
    hScm = OpenSCManager(0, 0, 1u);
    if (hScm)
    {

        hService = OpenService(hScm, lpServiceName, 0x20u);
        get_last_error();
        if (hService)
        {
            v6=ControlService(hService, 1u, &ServiceStatus);
            get_last_error();
            if (v6)
            {
                st = 1;
                status = L"完成";
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hScm);
    }
    else
    {
        MessageBox(0, L"无法获取服务控制管理器句柄", L"错误", 0x10u);
    }
    list_additem(lpServiceName, L"停止", status, g_error_code);
    return st;
}


int  sys_ccode(WCHAR *sys_name, DWORD dwIoControlCode)
{
    int st = 0;          // ebx
    WCHAR *status = 0;       // edi
    HANDLE hFile = 0;        // eax

    int v6 = 0;          // eax
    DWORD BytesReturned; // [esp+Ch] [ebp-108h] BYREF
    WCHAR FileName[260]; // [esp+10h] [ebp-104h] BYREF


    status = L"失败";
    if ((unsigned __int8)GetVersion() < 5u)
        wsprintfW(FileName, L"\\\\.\\%s", sys_name);
    else
        wsprintfW(FileName, L"\\\\.\\Global\\%s", sys_name);

    hFile=CreateFile(FileName, 0xC0000000, 0, 0, 3u, 0, 0);
    get_last_error();
    if (hFile ==INVALID_HANDLE_VALUE )
    {
        MessageBox(0, L"无法获取服务控制管理器句柄", L"错误", 0x10u);
    }
    else
    {
     
        v6=DeviceIoControl(hFile, dwIoControlCode, 0, 0, 0, 0, &BytesReturned, 0);
        get_last_error();
        if (v6)
        {
            st = 1;
            status = L"完成";
        }
        CloseHandle(hFile);
    }
    list_additem(sys_name, L"控制", status, g_error_code);
    return st;
}


int  is_sys(WCHAR *sysFileName, WCHAR *szDir)
{
    
    size_t i=0;
    WCHAR *exName;   // esi
    WCHAR *svc_name= wcsrchr(sysFileName,L'\\');
    if(svc_name==0)
    {
        MessageBox(0, L"路径错误", L"错误", 0x10u);
        return 0;
    }
    svc_name++;

    exName =wcsstr(svc_name,L".");
    if(exName==0)
    {
        MessageBox(0, L"没有发现扩展名", L"错误", 0x10u);
        return 0;
    } 

    while(1)
    {
        if(svc_name[i]==0 || svc_name[i]==46 || i>256)
        {
            szDir[i]=0;
            break;
        } 
        szDir[i]=svc_name[i];
        i++;
    }
    

    return 1;
}
