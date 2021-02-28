#pragma once

// CPageLocation

class CPageLocation : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageLocation)

public:
	CPageLocation();

	enum { IDD = IDD_PAGE_LOCATION };

	virtual ~CPageLocation();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
	CString m_strSourceFolder;
	CString m_strTargetFolder;
};


