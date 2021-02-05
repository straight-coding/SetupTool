#pragma once

#include "../include/ThreadBase.h"

#pragma warning(disable:4996)

class CInstallThread : public CThreadBase
{
private:
	int m_nResourceId;
	wchar_t m_szResourceName[MAX_PATH];
	wchar_t m_szOutputFile[MAX_PATH];

	UINT Runable()
	{
		CString strFormat;
		CInstallUtil* context = (CInstallUtil*)GetContext();

		strFormat.LoadStringW(IDS_EXTRACTING);
		Log(strFormat, L"");
		CInstallUtil::ExtractBinResource(m_szResourceName, m_nResourceId, m_szOutputFile);

		strFormat.LoadStringW(IDS_UNZIPING);
		Log(strFormat, L"");
		CString strOutPackFile = CInstallUtil::GetTempFile(L"unpack-", L".pack");
		BOOL bUnpackResult = CInstallUtil::UncompressFile(m_szOutputFile, strOutPackFile);
		context->AppendRecyle(m_szOutputFile);

		strFormat.LoadStringW(IDS_UNPACKING);
		Log(strFormat, L"");
		context->UnpackToFolder(strOutPackFile);

		context->Install();

		strFormat.LoadStringW(IDS_SUCCEEDED);
		Log(strFormat, L"");
		return 0;
	}
public:
	void SetUnzipInfo(const wchar_t* szResourceName, int nResourceId, const wchar_t* szOutputFile)
	{
		wcscpy(m_szResourceName, szResourceName);
		wcscpy(m_szOutputFile, szOutputFile);

		m_nResourceId = nResourceId;
	}
};

