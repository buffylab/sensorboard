// Get source from https://github.com/MadLittleMods/node-usb-detection
// Remove this code when libusb supports hotplug event notification on windows

#include <uv.h>
#include <dbt.h>
#include <iostream>
#include <iomanip>
// #include <atlstr.h>


// Include Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <Setupapi.h>
 
#include "android_usb.h"
#include "android_usb_win.h"

using namespace std;

/**********************************
 * Local defines
 **********************************/
#define VID_TAG "VID_"
#define PID_TAG "PID_"

#define LIBRARY_NAME ("setupapi.dll")


#define DllImport __declspec(dllimport)

#define MAX_THREAD_WINDOW_NAME 64

/**********************************
 * Local typedefs
 **********************************/



/**********************************
 * Local Variables
 **********************************/
GUID GUID_DEVINTERFACE_USB_DEVICE = {
	0xA5DCBF10L,
	0x6530,
	0x11D2,
	0x90,
	0x1F,
	0x00,
	0xC0,
	0x4F,
	0xB9,
	0x51,
	0xED
};

DWORD threadId;
HANDLE threadHandle;

HINSTANCE hinstLib; 


typedef BOOL (WINAPI *_SetupDiEnumDeviceInfo) (HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);
typedef HDEVINFO (WINAPI *_SetupDiGetClassDevs) (const GUID *ClassGuid, PCTSTR Enumerator, HWND hwndParent, DWORD Flags);
typedef BOOL (WINAPI *_SetupDiDestroyDeviceInfoList) (HDEVINFO DeviceInfoSet);
typedef BOOL (WINAPI *_SetupDiGetDeviceInstanceId) (HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, PTSTR DeviceInstanceId, DWORD DeviceInstanceIdSize, PDWORD RequiredSize);
typedef BOOL (WINAPI *_SetupDiGetDeviceRegistryProperty) (HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Property, PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize, PDWORD RequiredSize);


_SetupDiEnumDeviceInfo DllSetupDiEnumDeviceInfo;
_SetupDiGetClassDevs DllSetupDiGetClassDevs;
_SetupDiDestroyDeviceInfoList DllSetupDiDestroyDeviceInfoList;
_SetupDiGetDeviceInstanceId DllSetupDiGetDeviceInstanceId;
_SetupDiGetDeviceRegistryProperty DllSetupDiGetDeviceRegistryProperty;


/**********************************
 * Local Helper Functions protoypes
 **********************************/
DWORD WINAPI ListenerThread(LPVOID lpParam);

void LoadFunctions() {

	bool success;

	hinstLib = LoadLibrary(LIBRARY_NAME); 

	if (hinstLib != NULL) {
		DllSetupDiEnumDeviceInfo = (_SetupDiEnumDeviceInfo) GetProcAddress(hinstLib, "SetupDiEnumDeviceInfo"); 

		DllSetupDiGetClassDevs = (_SetupDiGetClassDevs) GetProcAddress(hinstLib, "SetupDiGetClassDevsA"); 

		DllSetupDiDestroyDeviceInfoList = (_SetupDiDestroyDeviceInfoList) GetProcAddress(hinstLib, "SetupDiDestroyDeviceInfoList"); 

		DllSetupDiGetDeviceInstanceId = (_SetupDiGetDeviceInstanceId) GetProcAddress(hinstLib, "SetupDiGetDeviceInstanceIdA"); 

		DllSetupDiGetDeviceRegistryProperty = (_SetupDiGetDeviceRegistryProperty) GetProcAddress(hinstLib, "SetupDiGetDeviceRegistryPropertyA"); 

		success = (
			DllSetupDiEnumDeviceInfo != NULL &&
			DllSetupDiGetClassDevs != NULL &&
			DllSetupDiDestroyDeviceInfoList != NULL &&
			DllSetupDiGetDeviceInstanceId != NULL &&
			DllSetupDiGetDeviceRegistryProperty != NULL
		);
	}
	else {
		success = false;
	}

	if(!success) {
		printf("Could not load library functions from dll -> abort (Check if %s is available)\r\n", LIBRARY_NAME);
		exit(1);
	}
}

void InitDetection() {

	LoadFunctions();

	threadHandle = CreateThread( 
			NULL,				// default security attributes
			0,					// use default stack size  
			ListenerThread,		// thread function name
			NULL,				// argument to thread function 
			0,					// use default creation flags 
			&threadId
		);
}

LRESULT CALLBACK DetectCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_DEVICECHANGE) {
		if ( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam ) {
			PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;

			if(pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
        // Detect event seems to be triggered before libusb can recognize new usb devices.
        // Wait one second fixes this problem, but this might not be sufficient for 
        // some device. 
        // Let's wait for libusb hotplug support : https://github.com/libusb/libusb/issues/86
        Sleep(1000);

        AndroidUsb::GetInstance()->RequestDiscover();
			}
		}
	}

	return 1;
}

DWORD WINAPI ListenerThread( LPVOID lpParam ) { 
	char className[MAX_THREAD_WINDOW_NAME];
	_snprintf_s(className, MAX_THREAD_WINDOW_NAME, "ListnerThreadUsbDetection_%d", GetCurrentThreadId());

	WNDCLASSA wincl = {0};
	wincl.hInstance = GetModuleHandle(0);
	wincl.lpszClassName = className;
	wincl.lpfnWndProc = DetectCallback;

	if (!RegisterClassA(&wincl)) {
		DWORD le = GetLastError();
		printf("RegisterClassA() failed [Error: %x]\r\n", le);
		return 1;
	}

	
	HWND hwnd = CreateWindowExA(WS_EX_TOPMOST, className, className, 0, 0, 0, 0, 0, NULL, 0, 0, 0);
	if (!hwnd) {
		DWORD le = GetLastError();
		printf("CreateWindowExA() failed [Error: %x]\r\n", le);
		return 1;
	}

	DEV_BROADCAST_DEVICEINTERFACE_A notifyFilter = {0};
	notifyFilter.dbcc_size = sizeof(notifyFilter);
	notifyFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	notifyFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

	HDEVNOTIFY hDevNotify = RegisterDeviceNotificationA(hwnd, &notifyFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (!hDevNotify) {
		DWORD le = GetLastError();
		printf("RegisterDeviceNotificationA() failed [Error: %x]\r\n", le);
		return 1;
	}

	MSG msg;
	while(TRUE) {
		BOOL bRet = GetMessage(&msg, hwnd, 0, 0);
		if ((bRet == 0) || (bRet == -1)) {
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}