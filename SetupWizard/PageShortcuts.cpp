// PageShortcuts.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageShortcuts.h"

//https://www.codeproject.com/Articles/29064/CGridListCtrlEx-Grid-Control-Based-on-CListCtrl
#include "..\CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include "..\CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "..\CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "..\CGridListCtrlEx\CGridColumnTraitHyperLink.h"
#include "..\CGridListCtrlEx\CGridRowTraitXP.h"
#include "..\CGridListCtrlEx\ViewConfigSection.h"

// CPageShortcuts

//IMPLEMENT_DYNAMIC(CPageShortcuts, CPropertyPage)

CPageShortcuts::CPageShortcuts() : CPropertyPage(CPageShortcuts::IDD)
{

}

CPageShortcuts::~CPageShortcuts()
{
}

BEGIN_MESSAGE_MAP(CPageShortcuts, CPropertyPage)
END_MESSAGE_MAP()

// CPageShortcuts message handlers

BOOL CPageShortcuts::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
	int nStateImageIdx = CGridColumnTraitDateTime::AppendStateImages(m_ListCtrl, m_ImageList);	// Add checkboxes
	m_ListCtrl.SetImageList(&m_ImageList, LVSIL_SMALL);

	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);
	m_ListCtrl.EnableVisualStyles(true);

	// Create Columns
	m_ListCtrl.InsertHiddenLabelColumn();	// Requires one never uses column 0

	for (int col = 0; col < 3; ++col)
	{
		CString strTitle;
		CGridColumnTrait* pTrait = NULL;
		if (col == 0)
		{
			strTitle = L"Application";
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			//const vector<CString>& countries = m_DataModel.GetCountries();
			//for (size_t i = 0; i < countries.size(); ++i)
				//pComboTrait->AddItem((DWORD_PTR)i, countries[i]);
			pTrait = pComboTrait;
		}
		else if (col == 1)
		{
			strTitle = L"Start Menu";
			pTrait = new CGridColumnTraitEdit;
		}
		else if (col == 2)
		{
			strTitle = L"Desktop";
			CGridColumnTraitDateTime* pDateTimeTrait = new CGridColumnTraitDateTime;
			pDateTimeTrait->AddImageIndex(-1, _T(""), false);		// Unchecked (and not editable)
			//pDateTimeTrait->SetToggleSelection(true);
			pTrait = pDateTimeTrait;
		}

		m_ListCtrl.InsertColumnTrait(col + 1, strTitle, LVCFMT_LEFT, 100, col, pTrait);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageShortcuts::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	CStringArray aryFileList;
	CString strFolder = GetValue(L"source_folder");
	if ((strFolder.Right(1) != "\\") && (strFolder.Right(1) != "\\"))
		strFolder += "\\";

	int nPrefixLen = strFolder.GetLength();
	CInstallUtil::ScanFolder(strFolder, L"*.exe", aryFileList);

	CStringArray aryExeList;
	for (int i = 0; i < aryFileList.GetCount(); i++)
	{
		TRACE(L"%s\r\n", aryFileList[i]);
		CString strPath = aryFileList[i].Mid(nPrefixLen);
		aryExeList.Add(strPath);
	}

	int nItem = 0;
	m_ListCtrl.DeleteAllItems();
	for (int row = 0; row < aryExeList.GetCount(); ++row)
	{
		nItem = m_ListCtrl.InsertItem(++nItem, aryExeList[row]);
		//m_ListCtrl.SetItemData(nItem, rowId);
		for (int col = 0; col < 3; ++col)
		{
			int nCellCol = col + 1;	// +1 because of hidden column
			//const CString& strCellText = m_DataModel.GetCellText(rowId, col);
			if (nCellCol == 1)
				m_ListCtrl.SetItemText(nItem, nCellCol, aryExeList[row]);
			else if (nCellCol > 1)
			{
				//if (strCellText == _T(""))
					//m_ListCtrl.SetCellImage(nItem, nCellCol, nStateImageIdx);	// unchecked
				//else
				m_ListCtrl.SetCellImage(nItem, nCellCol, 1);	// checked
			}
		}
		//m_ListCtrl.SetCellImage(nItem, 1, nItem); // Assign flag-images
	}

	return CPropertyPage::OnSetActive();
}

BOOL CPageShortcuts::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}


void CPageShortcuts::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SHORTCUTS, m_ListCtrl);
}
