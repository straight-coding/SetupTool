// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

map<CString, CString> g_mapPairs;
vector<CString> g_listCheckRunning;

void SetupDefaultPairs()
{
	SetValue(L"product_name", L"Straight");
	SetValue(L"product_version", L"V1.0.0.0");
	SetValue(L"publisher", L"Straight Coder");
	SetValue(L"platform", L"x86");
	SetValue(L"language", L"en");
	SetValue(L"install_folder", L"%SystemDrive%/%programfiles(x86)%/%product_name%");
	SetValue(L"source_folder", L"C:\\Program Files\\Windows NT\\Accessories");
}

void SetValue(CString key, CString value)
{
	g_mapPairs[key] = value;
}

CString GetValue(CString key)
{
	if (g_mapPairs.find(key) != g_mapPairs.end())
		return g_mapPairs[key];
	return L"";
}

vector<CString> GetCheckList()
{
	return g_listCheckRunning;
}

void ClearCheckList()
{
	g_listCheckRunning.clear();
}

void AppendCheckList(CString strPath)
{
	if (std::find(g_listCheckRunning.begin(), g_listCheckRunning.end(), strPath) != g_listCheckRunning.end())
		return;

	g_listCheckRunning.push_back(strPath);
}

void RemoveFromCheckList(CString strPath)
{
	vector<CString>::iterator iter = std::find(g_listCheckRunning.begin(), g_listCheckRunning.end(), strPath);
	if (iter != g_listCheckRunning.end())
		g_listCheckRunning.erase(iter);
}
