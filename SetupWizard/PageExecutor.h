#pragma once

// CPageExecutor

class CPageExecutor : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageExecutor)

public:
	CPageExecutor();

	enum { IDD = IDD_PAGE_EXECUTOR };

	virtual ~CPageExecutor();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
};


