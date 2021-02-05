/*
  UninstallThead.h
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#pragma once

#include "../include/ThreadBase.h"

class CUninstallThread : public CThreadBase
{
private:
	UINT Runable()
	{
		CString strFormat;
		CInstallUtil* context = (CInstallUtil*)GetContext();

		context->Uninstall();

		strFormat.LoadStringW(IDS_SUCCEEDED);
		Log(strFormat, L"");
		return 0;
	}
};
