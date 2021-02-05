
// Uninstaller.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "Uninstaller.h"
#include "UninstallerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUninstallerApp

BEGIN_MESSAGE_MAP(CUninstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CUninstallerApp construction

CUninstallerApp::CUninstallerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CUninstallerApp object

CUninstallerApp theApp;

#pragma warning(disable:4996)

// CUninstallerApp initialization

BOOL CUninstallerApp::InitInstance()
{
	CWinApp::InitInstance();

	try
	{
		BOOL fIsRunAsAdmin;
		fIsRunAsAdmin = CInstallUtil::IsRunAsAdmin();
		if (!fIsRunAsAdmin)
		{
			wchar_t szPath[MAX_PATH];
			if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
			{
				wcscpy(szPath, CInstallUtil::GetCmdLine());

				SHELLEXECUTEINFO sei = { sizeof(sei) };
				sei.lpVerb = L"runas";
				sei.lpFile = szPath;
				sei.lpParameters = m_lpCmdLine;
				//sei.hwnd = hWnd;
				sei.nShow = SW_NORMAL;

				if (!ShellExecuteEx(&sei))
				{
					DWORD dwError = GetLastError();
					if (dwError == ERROR_CANCELLED)
					{
						return FALSE;
					}
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
	catch (CException* e)
	{
		e->Delete();
	}

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CUninstallerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

