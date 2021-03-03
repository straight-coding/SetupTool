#pragma once

// CPageCheckRunning

class CPageCheckRunning : public CPropertyPage
{
	//DECLARE_DYNAMIC(CPageCheckRunning)

public:
	CPageCheckRunning();

	enum { IDD = IDD_PAGE_CHECK_RUNNING };

	virtual ~CPageCheckRunning();

	void ReloadList();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();

	CListBox m_ctrlSrcFiles;
	CListBox m_ctrlCheckFiles;
	afx_msg void OnLbnSelchangeListFilesCheck();
	afx_msg void OnLbnSelchangeListFilesSrc();
	afx_msg void OnLbnDblclkListFilesSrc();
	afx_msg void OnLbnDblclkListFilesCheck();
};


