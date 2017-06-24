/****************************************************
	Project name:	ScyllaHideWindbgPlugin
	Date:			19.02.2015
	Purpose:		Windbg plugin for ScyllaHide
****************************************************/

#pragma once

/* Includes **************************************************/

#include <Windows.h>
#include <DbgEng.h>

/* Declarations **********************************************/

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define __in_req __in __notnull
#define TITANHIDE	("\\\\.\\TitanHide")

#define PROCESS_DEBUG_FLAGS			(L"ProcessDebugFlags")
#define PROCESS_DEBUG_PORT			(L"ProcessDebugPort")
#define PROCESS_DEBUG_OBJECT_HANDLE	(L"ProcessDebugObjectHandle")
#define DEBUG_OBJECT				(L"DebugObject")
#define SYSTEM_DEBUG_INFORAMTION	(L"SystemDebuggerInformation")
#define NT_CLOSE					(L"NtClose")
#define THREAD_HIDE_FROM_DEBUGGER	(L"ThreadHideFromDebugger")
#define SET_CONTEXT_THREAD			(L"SetContextThread")

/* Definitions ***********************************************/

typedef IDebugClient	* PIDebugClient;
typedef IDebugControl	* PIDebugControl;
typedef IDebugSymbols	* PIDebugSymbols;

/* Functions *************************************************/

/*************************************************
	Function name:	DebugExtensionInitialize
	Paramters:		Version		[out]	Pointer to the extention version
					Flags		[out]	Must be 0 - legacy 
	Purpose:		DebugExtensionInitialize is called when the extention is loaded
	Date:			25/12/2015
	Author:			Dvir Atias
**************************************************/
HRESULT
CALLBACK
DebugExtensionInitialize(
	__out PULONG Version,
	__out PULONG Flags
);

/*************************************************
	Function name:	DebugExtensionUninitialize
	Purpose:		DebugExtensionUninitialize is called when the extention is unloaded
	Date:			25/12/2015
	Author:			Dvir Atias
**************************************************/
VOID 
CALLBACK
DebugExtensionUninitialize(
);

