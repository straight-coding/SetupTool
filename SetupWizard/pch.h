// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#pragma warning(disable:4996)

#include "framework.h"

#include <algorithm>
#include <map>
#include <vector>

using namespace std;

#include "../include/InstallUtil.h"

void SetupDefaultPairs();
void SetValue(CString key, CString value);
CString GetValue(CString key);

vector<CString> GetCheckList();
void ClearCheckList();
void AppendCheckList(CString strPath);
void RemoveFromCheckList(CString strPath);

#endif //PCH_H
