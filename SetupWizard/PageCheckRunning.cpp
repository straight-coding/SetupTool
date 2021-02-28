// PageCheckRunning.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageCheckRunning.h"

// CPageCheckRunning

//IMPLEMENT_DYNAMIC(CPageCheckRunning, CPropertyPage)

CPageCheckRunning::CPageCheckRunning() : CPropertyPage(CPageCheckRunning::IDD)
{
}

CPageCheckRunning::~CPageCheckRunning()
{
}

BEGIN_MESSAGE_MAP(CPageCheckRunning, CPropertyPage)
END_MESSAGE_MAP()

// CPageCheckRunning message handlers

BOOL CPageCheckRunning::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageCheckRunning::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	//sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	//sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	//sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	return CPropertyPage::OnSetActive();
}

BOOL CPageCheckRunning::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}


void CPageCheckRunning::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
}
