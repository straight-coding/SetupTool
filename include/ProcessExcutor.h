/*
  ProcessExcutor.h
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#ifndef _PROCESS_EXCUTOR_
#define _PROCESS_EXCUTOR_

#include <string>
#include <vector>
#include <map>

using namespace std;

#include <afxwin.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "ThreadBase.h"

#pragma warning(disable:4996)

class CInstallUtil;
class CProcessExcutor : public CThreadBase
{
public:
	CProcessExcutor()
	{
		m_bVisible = FALSE;

		m_bRedirectIO = FALSE;
		m_bWaitFinish = FALSE;

		m_hChildStd_IN_Rd = NULL;
		m_hChildStd_IN_Wr = NULL;
		m_hChildStd_OUT_Rd = NULL;
		m_hChildStd_OUT_Wr = NULL;

		memset(&m_StartupInfo, 0, sizeof(STARTUPINFO));
		memset(&m_ProcInfo, 0, sizeof(PROCESS_INFORMATION));
		m_strLogCache = L"";
	}

private:
	BOOL m_bVisible;

	STARTUPINFOW		m_StartupInfo;
	PROCESS_INFORMATION m_ProcInfo;

	BOOL   m_bRedirectIO;
	BOOL   m_bWaitFinish;

	HANDLE m_hChildStd_IN_Rd;
	HANDLE m_hChildStd_IN_Wr;
	HANDLE m_hChildStd_OUT_Rd;
	HANDLE m_hChildStd_OUT_Wr;

	CString m_strLogCache;

	CString m_strCommand;
	CString m_strParameters;

	static CString FormatCommandLine(const wchar_t* szCommandLine)
	{
		CString strReformated = L"";

		int nArgs = 0;
		LPWSTR* szArglist = CommandLineToArgvW(szCommandLine, &nArgs);
		if (NULL != szArglist)
		{
			for (int i = 0; i < nArgs; i++)
			{
				CString str = szArglist[i];
				if ((str.Mid(1, 2) == L":\\") || 
					(wcsicmp(str.Left(5), L"HKEY_") == 0) ||
					(str.Mid(1, 2) == L":/"))
				{
					str.Replace(L"/", L"\\");
					str = L"\"" + str + L"\"";
				}

				if (i > 0)
					strReformated += L" ";
				strReformated += str;
			}
			LocalFree(szArglist);
		}

		return strReformated;
	}

public:

	void SetVisible(BOOL bVisible)
	{
		m_bVisible = bVisible;
	}

	void SetRedirect(BOOL bRedirect)
	{
		m_bRedirectIO = bRedirect;
	}

	void SetWait(BOOL bWait)
	{
		m_bWaitFinish = bWait;
	}

	void SetCommand(const wchar_t* szCommand, const wchar_t* szParameters)
	{
		m_strCommand = szCommand;
		m_strParameters = szParameters;
	}

	UINT Runable()
	{
		SECURITY_ATTRIBUTES   saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.lpSecurityDescriptor = NULL;
		saAttr.bInheritHandle = TRUE;

		if (m_bRedirectIO)
		{
			m_bWaitFinish = TRUE;

			// Create a pipe for the child process's STDOUT. 
			if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
			{
				m_hChildStd_OUT_Rd = NULL;
				m_hChildStd_OUT_Wr = NULL;
			}

			// Ensure the read handle to the pipe for STDOUT is not inherited.
			if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
			{
				CloseHandle(m_hChildStd_OUT_Rd);
				CloseHandle(m_hChildStd_OUT_Wr);
				m_hChildStd_OUT_Rd = NULL;
				m_hChildStd_OUT_Wr = NULL;
			}

			// Create a pipe for the child process's STDIN. 
			if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))
			{
				m_hChildStd_IN_Rd = NULL;
				m_hChildStd_IN_Wr = NULL;
			}

			// Ensure the write handle to the pipe for STDIN is not inherited. 
			if (!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
			{
				CloseHandle(m_hChildStd_IN_Rd);
				CloseHandle(m_hChildStd_IN_Wr);
				m_hChildStd_IN_Rd = NULL;
				m_hChildStd_IN_Wr = NULL;
			}
		}

		ZeroMemory(&m_ProcInfo, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&m_StartupInfo, sizeof(STARTUPINFOW));

		m_StartupInfo.cb = sizeof(STARTUPINFO);
		m_StartupInfo.lpReserved = NULL;
		m_StartupInfo.lpDesktop = NULL;
		m_StartupInfo.lpTitle = NULL;
		m_StartupInfo.dwX = 0;
		m_StartupInfo.dwY = 0;
		m_StartupInfo.dwXSize = 0;
		m_StartupInfo.dwYSize = 0;
		m_StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		m_StartupInfo.wShowWindow = m_bVisible ? SW_SHOW : SW_HIDE;
		m_StartupInfo.cbReserved2 = 0;
		m_StartupInfo.lpReserved2 = NULL;

		m_StartupInfo.hStdError = m_hChildStd_OUT_Wr;
		m_StartupInfo.hStdOutput = m_hChildStd_OUT_Wr;
		m_StartupInfo.hStdInput = m_hChildStd_IN_Rd;
		m_StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

		wchar_t szCommandLine[MAX_PATH];
		wsprintf(szCommandLine, L"%s %s", m_strCommand, m_strParameters);
		CString strCommand = FormatCommandLine(szCommandLine);
		wcscpy(szCommandLine, strCommand);

		BOOL bResult = CreateProcessW(NULL, szCommandLine, &saAttr, &saAttr, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &m_StartupInfo, &m_ProcInfo);
		while (m_bWaitFinish)
		{
			BOOL bEnded = (WaitForSingleObject(m_ProcInfo.hProcess, 0) == WAIT_OBJECT_0);
			if (m_hChildStd_OUT_Rd != NULL)
			{
				DWORD dwAvail = 0;
				if (!PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &dwAvail, NULL))
				{
					if (bEnded)
						break;
					Sleep(10);
					continue;
				}

				if (!dwAvail)
				{
					if (bEnded)
						break;
					Sleep(10);
					continue;
				}
				else
				{
					char chBuf[512];
					memset(chBuf, 0, sizeof(chBuf));

					dwAvail = 0;
					ReadFile(m_hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 2, &dwAvail, NULL);
					if (dwAvail > 0)
						ParseOutput(chBuf);
				}

				if (bEnded)
					break;
			}
			Sleep(20);
		}

		if (m_hChildStd_OUT_Rd != NULL)		CloseHandle(m_hChildStd_OUT_Rd);
		if (m_hChildStd_OUT_Wr != NULL)		CloseHandle(m_hChildStd_OUT_Wr);
		if (m_hChildStd_IN_Rd != NULL)		CloseHandle(m_hChildStd_IN_Rd);
		if (m_hChildStd_IN_Wr != NULL)		CloseHandle(m_hChildStd_IN_Wr);

		m_hChildStd_OUT_Rd = NULL;
		m_hChildStd_OUT_Wr = NULL;
		m_hChildStd_IN_Rd = NULL;
		m_hChildStd_IN_Wr = NULL;

		return 0L;
	}

	//to wait external process
	BOOL IsProcessRunning(DWORD pid)
	{ //const wchar_t *processName, 
		BOOL exists = false;
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (Process32First(snapshot, &entry))
		{
			while (Process32Next(snapshot, &entry))
			{
				//if (!wcsicmp(entry.szExeFile, processName))
				if (entry.th32ProcessID == pid)
				{
					exists = true;
					break;
				}
			}
		}

		CloseHandle(snapshot);
		return exists;
	}

	//to wait external process
	BOOL IsProcessRunningEx(DWORD pid)
	{
		HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
		DWORD ret = WaitForSingleObject(process, 0);
		CloseHandle(process);
		return (ret == WAIT_TIMEOUT);
	}

	//to wait external process
	BOOL IsProcessAlive(DWORD dwPID)
	{
		if ((dwPID != 0))
		{
			DWORD dwProcesses[2048], cbSizeNeeded;
			if (EnumProcesses(dwProcesses, sizeof(dwProcesses), &cbSizeNeeded))
			{
				DWORD cProcesses = cbSizeNeeded / sizeof(DWORD);
				for (DWORD i = 0; i < cProcesses; i++)
				{
					if (dwProcesses[i] == dwPID)
						return TRUE;
				}
			}
		}
		return FALSE;
	}

	//to wait external process
	void ParseOutput(char* pBuf)
	{
		if (pBuf[0] == 0)
			return;

		m_strLogCache += pBuf;
		while (m_strLogCache.GetLength() > 0)
		{
			int idx = m_strLogCache.FindOneOf(L"\r\n");
			if (idx < 0)
				break;

			CString strLine = m_strLogCache.Left(idx);
			m_strLogCache = m_strLogCache.Mid(idx + 1);

			if (strLine.IsEmpty())
				continue;

			Log(L"%s\r\n", strLine);
		}
	}
};

#endif
