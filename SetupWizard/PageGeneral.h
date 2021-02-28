#pragma once

// CPageGeneral

class CPageGeneral : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageGeneral)

public:
	CPageGeneral();

	enum { IDD = IDD_PAGE_GENERAL };

	virtual ~CPageGeneral();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
	CString m_strProductName;
	CString m_strPublisher;
	CComboBox m_ctrlPlatform;
	CComboBox m_ctrlLanguage;
	CString m_strProductVersion;
};


