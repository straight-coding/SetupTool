#pragma once


// CPageCreater

class CPageCreater : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageCreater)

public:
	CPageCreater();

	enum { IDD = IDD_PAGE_CREATE };

	virtual ~CPageCreater();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
};


