#include "TitanHideWindbgPlugin.h"
#include "..\TitanHide\TitanHide.h"

static PIDebugClient	gp_IDebugClient		= NULL;
static PIDebugControl	gp_IDebugControl	= NULL;
static PIDebugSymbols	gp_IDebugSymbols	= NULL;
static HANDLE			g_hDriverHandle		= INVALID_HANDLE_VALUE;
static BOOL				g_bIsErrorOccured	= FALSE;

static 
BOOL
CALLBACK
TitanHide(
	__in_req HIDE_INFO * ptHideInfo
)
{
	DWORD	dwBytesWritten		= 0;
	BOOL	bIsWriteSuccssed	= FALSE;

	if (NULL == ptHideInfo)
	{
		goto Cleanup;
	}

	bIsWriteSuccssed = WriteFile(g_hDriverHandle, ptHideInfo, sizeof(*ptHideInfo), &dwBytesWritten, NULL);
	if (!bIsWriteSuccssed)
	{
		goto Cleanup;
	}

	if (sizeof(*ptHideInfo) != dwBytesWritten)
	{
		goto Cleanup;
	}

	bIsWriteSuccssed = TRUE;
Cleanup:
	return bIsWriteSuccssed;
}

static
BOOL
CALLBACK
ParseCommandArgA(
	__in_req	PCSTR	pszArg,
	__out		PWSTR ** pppwzParsedArg,
	__out		PINT	 piArgc
)
{
	DWORD	dwArgSize			= 0;
	PWSTR	pwszArgument		= NULL;
	PWSTR * ppwszParsedArg		= NULL;
	INT		iArgc				= 0;
	BOOL	bIsParseSuccessed	= FALSE;
	BOOL	bIsArgAllocated		= FALSE;

	if (NULL == pszArg || NULL == pppwzParsedArg)
	{
		goto Cleanup;
	}

	dwArgSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pszArg, -1, NULL, 0);
	if (0 == dwArgSize)
	{
		goto Cleanup;
	}
	
	pwszArgument = (PWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwArgSize * sizeof(WCHAR));
	if (NULL == pwszArgument)
	{
		goto Cleanup;
	}

	bIsArgAllocated = TRUE;

	dwArgSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pszArg, -1, pwszArgument, dwArgSize);
	if (0 == dwArgSize)
	{
		goto Cleanup;
	}

	ppwszParsedArg = CommandLineToArgvW(pwszArgument, &iArgc);
	if (NULL == ppwszParsedArg)
	{	
		goto Cleanup;
	}

	*pppwzParsedArg = ppwszParsedArg;
	*piArgc = iArgc;
	bIsParseSuccessed = TRUE;
Cleanup:
	if (bIsArgAllocated && !bIsParseSuccessed)
	{
		HeapFree(GetProcessHeap(), 0, pwszArgument);
	}

	return bIsParseSuccessed;
}

HRESULT 
CALLBACK
DebugExtensionInitialize(
	__out PULONG Version,
	__out PULONG Flags
)
{
	HRESULT hrResult = S_FALSE;

	if (NULL == Version || NULL == Flags)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "DebugExtensionInitialize: One of the parameters are NULL\n");
		g_bIsErrorOccured = TRUE;
		goto Cleanup;
	}
	
	hrResult = DebugCreate(__uuidof (IDebugClient), (PVOID *)&gp_IDebugClient);
	if (FAILED(hrResult))
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "DebugExtensionInitialize: Could not get IDebugClient interface\n");
		g_bIsErrorOccured = TRUE;
		goto Cleanup;
	}

	hrResult = gp_IDebugClient->QueryInterface(_uuidof(IDebugControl), (PVOID *)&gp_IDebugControl);
	if (FAILED(hrResult))
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "DebugExtensionInitialize: Could not get IDebugControl interface\n");
		g_bIsErrorOccured = TRUE;
		goto Cleanup;
	}

	hrResult = gp_IDebugClient->QueryInterface(_uuidof(IDebugSymbols), (PVOID *)&gp_IDebugSymbols);
	if (FAILED(hrResult))
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "DebugExtensionInitialize: Could not get IDebugSymbols interface\n");
		g_bIsErrorOccured = TRUE;
		goto Cleanup;
	}

	g_hDriverHandle = CreateFileA(TITANHIDE, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == g_hDriverHandle)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "DebugExtensionInitialize: TitanHide is not loaded!\n");
		g_bIsErrorOccured = TRUE;
		goto Cleanup;
	}

	*Version = DEBUG_EXTENSION_VERSION(MAJOR_VERSION, MINOR_VERSION);
	*Flags = 0;
	hrResult = S_OK;
Cleanup:
	return hrResult;
}

VOID
CALLBACK
DebugExtensionUninitialize(
)
{
	if (INVALID_HANDLE_VALUE != g_hDriverHandle)
	{
		CloseHandle(g_hDriverHandle);
	}

	OutputDebugString(L"DebugExtensionUninitialize: TitanHide is unloaded..\n");
}

HRESULT
CALLBACK
Help(
	__in_req PDEBUG_CLIENT Client,
	__in_req PCSTR args
)
{
	HRESULT hrResult	= S_FALSE;
	BYTE pszHelp[]		= "TitanHide is a kernel mode anti anti debugging plugin. Before using TitanHide plugin make sure TitanHide driver installed.\n!TitanHide.Hide PID, [func1, func2..] - Anti anti debug all listed WINAPI functions\n!TitanHide.Unhide  PID, [func1, func2..] - Stop anti anti debug all listed WINAPI functions\n!TitanHide.List - List all the supported anti anti debug techniques\n!TitanHide.Help - Help menu\n";
	
	if (g_bIsErrorOccured)
	{
		goto Cleanup;
	}

	if (NULL == Client || NULL == args)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Help: One of the parameters are NULL\n");
		goto Cleanup;
	}
	
	hrResult = gp_IDebugControl->Output(DEBUG_OUTPUT_NORMAL, (PCSTR)pszHelp);
Cleanup:
	return hrResult;
}

HRESULT
CALLBACK
List(
	__in_req PDEBUG_CLIENT Client,
	__in_req PCSTR args
)
{
	HRESULT hrResult	= S_FALSE;
	BYTE	pszHelp[]	= "1. ProcessDebugFlags\n2. ProcessDebugPort\n3. ProcessDebugObjectHandle\n4. DebugObject\n5. SystemDebuggerInformation\n6. NtClose\n7. ThreadHideFromDebugger\n8. SetContextThread\n";

	if (g_bIsErrorOccured)
	{
		goto Cleanup;
	}

	if (NULL == Client || NULL == args)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "List: One of the parameters are NULL\n");
		goto Cleanup;
	}

	hrResult = gp_IDebugControl->Output(DEBUG_OUTPUT_NORMAL, (PCSTR)pszHelp);

Cleanup:
	return hrResult;
}

HRESULT
CALLBACK
Hide(
	__in_req PDEBUG_CLIENT Client,
	__in_req PCSTR args
)
{
	HRESULT		hrResult				= S_FALSE;
	HIDE_INFO 	tHideInfo				= { (HIDE_COMMAND) 0, 0, 0};
	HIDE_TYPE	eHideType				= (HIDE_TYPE) 0;
	INT			iArgc					= 0;
	INT			iArgCount				= 1;
	PWSTR	*	pwszCommandArgv			= NULL;
	BOOL		bIsParseCmdSuccssed		= NULL;
	BOOL		bIsTitanHideSuccssed	= NULL;
	SIZE_T		iPid					= 0;
	
	if (g_bIsErrorOccured)
	{
		goto Cleanup;
	}

	if (NULL == Client || NULL == args)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Hide: One of the parameters are NULL\n");
		goto Cleanup;
	}

	bIsParseCmdSuccssed = ParseCommandArgA(args, &pwszCommandArgv, &iArgc);
	if (!bIsParseCmdSuccssed)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Hide: The parameters passed to Hide are invalid\n");
		goto Cleanup;
	}
		
	iPid = (SIZE_T) _wtoi64(pwszCommandArgv[0]);
	if (0 == iPid)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Hide: Invalid pid\n");
		goto Cleanup;
	}

	tHideInfo.Command	= HidePid;
	tHideInfo.Pid		= (ULONG) iPid;

	for (; iArgCount < iArgc; iArgCount++)
	{
		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_FLAGS))
		{
			tHideInfo.Type |= HideProcessDebugFlags;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_PORT))
		{
			tHideInfo.Type |= HideProcessDebugPort;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_OBJECT_HANDLE))
		{
			tHideInfo.Type |= HideProcessDebugObjectHandle;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], SYSTEM_DEBUG_INFORAMTION))
		{
			tHideInfo.Type |= HideSystemDebuggerInformation;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], NT_CLOSE))
		{
			tHideInfo.Type |= HideNtClose;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], THREAD_HIDE_FROM_DEBUGGER))
		{
			tHideInfo.Type |= HideThreadHideFromDebugger;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], SET_CONTEXT_THREAD))
		{
			tHideInfo.Type |= HideNtSetContextThread;
		}
	}

	bIsTitanHideSuccssed = TitanHide(&tHideInfo);
	if (!bIsTitanHideSuccssed)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Hide: TitanHide failed to work\n");
		goto Cleanup;
	}

	gp_IDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Hide: TitanHide is enabled!\n");
	hrResult = S_OK;
Cleanup:
	if (bIsParseCmdSuccssed)
	{
		HeapFree(GetProcessHeap(), 0, *pwszCommandArgv);
	}

	return hrResult;
}

HRESULT
CALLBACK
Unhide(
	__in_req PDEBUG_CLIENT Client,
	__in_req PCSTR args
)
{
	HRESULT		hrResult				= S_FALSE;
	HIDE_INFO 	tHideInfo				= { (HIDE_COMMAND)0, 0, 0 };
	HIDE_TYPE	eHideType				= (HIDE_TYPE)0;
	INT			iArgc					= 0;
	INT			iArgCount				= 1;
	PWSTR	*	pwszCommandArgv			= NULL;
	BOOL		bIsParseCmdSuccssed		= NULL;
	BOOL		bIsTitanHideSuccssed	= NULL;
	SIZE_T		iPid					= 0;

	if (g_bIsErrorOccured)
	{
		goto Cleanup;
	}

	if (NULL == Client || NULL == args)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Unhide: One of the parameters are NULL\n");
		goto Cleanup;
	}

	bIsParseCmdSuccssed = ParseCommandArgA(args, &pwszCommandArgv, &iArgc);
	if (!bIsParseCmdSuccssed)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Unhide: The parameters passed to Hide are invalid\n");
		goto Cleanup;
	}

	iPid = (SIZE_T) _wtoi64(pwszCommandArgv[0]);
	if (0 == iPid)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "UnHide: Invalid pid\n");
		goto Cleanup;
	}

	tHideInfo.Command	= HidePid;
	tHideInfo.Pid		= (ULONG) iPid;

	tHideInfo.Command = UnhidePid;
	tHideInfo.Pid = GetCurrentProcessId();

	for (; iArgCount < iArgc; iArgCount++)
	{
		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_FLAGS))
		{
			tHideInfo.Type |= HideProcessDebugFlags;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_PORT))
		{
			tHideInfo.Type |= HideProcessDebugPort;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], PROCESS_DEBUG_OBJECT_HANDLE))
		{
			tHideInfo.Type |= HideProcessDebugObjectHandle;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], SYSTEM_DEBUG_INFORAMTION))
		{
			tHideInfo.Type |= HideSystemDebuggerInformation;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], NT_CLOSE))
		{
			tHideInfo.Type |= HideNtClose;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], THREAD_HIDE_FROM_DEBUGGER))
		{
			tHideInfo.Type |= HideThreadHideFromDebugger;
		}

		if (0 == lstrcmpW(pwszCommandArgv[iArgCount], SET_CONTEXT_THREAD))
		{
			tHideInfo.Type |= HideNtSetContextThread;
		}
	}

	bIsTitanHideSuccssed = TitanHide(&tHideInfo);
	if (!bIsTitanHideSuccssed)
	{
		gp_IDebugControl->Output(DEBUG_OUTPUT_ERROR, "Unhide: TitanHide failed to work\n");
		goto Cleanup;
	}

	gp_IDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Unhide: TitanHide is enabled!\n");
	hrResult = S_OK;
Cleanup:
	if (bIsParseCmdSuccssed)
	{
		HeapFree(GetProcessHeap(), 0, *pwszCommandArgv);
	}

	return hrResult;

}