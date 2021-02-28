// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.
void SetupDefaultPairs()
{
	SetValue(L"product_name", L"Straight");
	SetValue(L"product_version", L"V1.0.0.0");
	SetValue(L"publisher", L"Straight Coder");
	SetValue(L"platform", L"x86");
	SetValue(L"language", L"en");
	SetValue(L"install_folder", L"%SystemDrive%/%programfiles(x86)%/%install_name%");
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
