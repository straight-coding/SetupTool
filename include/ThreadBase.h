#pragma once

#include <afxwin.h>

#pragma warning(disable:4996)

class CThreadBase
{
public:
	CThreadBase()
	{
		m_pThread = NULL;
		m_pContext = NULL;

		m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hStopped = CreateEvent(NULL, TRUE, TRUE, NULL);

		m_Logger = NULL;
	}

	~CThreadBase()
	{
		CloseHandle(m_hStopEvent);
		CloseHandle(m_hStopped);
	}

private:
	CMutex m_Lock;

	HANDLE	m_hStopEvent;
	HANDLE	m_hStopped;

	CWinThread* m_pThread;
	void* m_pContext;
	void (*m_Logger)(CString strFormat, ...);

	void Finished()
	{
		SetEvent(m_hStopped);
		ResetEvent(m_hStopEvent);
	}

	static UINT WorkerThread(LPVOID pContext)
	{
		CThreadBase* worker = (CThreadBase*)pContext;

		UINT result = worker->Runable();
		worker->Finished();

		return result;
	}

protected:

	virtual UINT Runable() = 0;

public:

	void* GetContext()
	{
		return m_pContext;
	}

	void SetLogger(void (*logger)(CString, ...))
	{
		m_Logger = logger;
	}

	void Log(CString strFormat, ...)
	{
		if (m_Logger != NULL)
		{
			CString s;

			va_list argList;
			va_start(argList, strFormat);
			s.FormatV(strFormat, argList);
			va_end(argList);

			m_Logger(s);
		}
	}

	HANDLE GetThreadHandle()
	{
		return m_pThread->m_hThread;
	}

	BOOL IsStopping()
	{
		BOOL bStopping = (WaitForSingleObject(m_hStopEvent, 0) != WAIT_TIMEOUT);
		return bStopping;
	}

	BOOL IsStopped()
	{
		BOOL bStopped = (WaitForSingleObject(m_hStopped, 0) != WAIT_TIMEOUT);
		return bStopped;
	}

	BOOL Start(void* context)
	{
		m_pContext = context;

		if (!IsStopped())
			return FALSE;

		m_pThread = AfxBeginThread(WorkerThread, (VOID*)this);
		if (m_pThread != NULL)
		{
			m_pThread->m_bAutoDelete = TRUE;
			ResetEvent(m_hStopped);
		}

		return (NULL != m_pThread);
	}

	void Stop()
	{
		SetEvent(m_hStopEvent);
		while (!IsStopped())
		{
			Sleep(10);
		}
	}
};
