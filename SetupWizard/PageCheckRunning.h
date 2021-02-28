#pragma once

// CPageCheckRunning

class CPageCheckRunning : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageCheckRunning)

public:
	CPageCheckRunning();

	enum { IDD = IDD_PAGE_CHECK_RUNNING };

	virtual ~CPageCheckRunning();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
};


