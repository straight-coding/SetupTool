#pragma once

// CPageShortcuts

class CPageShortcuts : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageShortcuts)

public:
	CPageShortcuts();

	enum { IDD = IDD_PAGE_SHORTCUTS };

	virtual ~CPageShortcuts();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);
};


