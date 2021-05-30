/*
  InstallUtil.h
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#ifndef _INSTALL_UTIL_
#define _INSTALL_UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <stdarg.h>

#include <Wincrypt.h>
#include <psapi.h>
#include <wbemidl.h>

#include <string>
#include <vector>
#include <map>

#include <fstream>
#include <iostream>
#include <cstdio>

using namespace std;

#include "../zlib/zlib.h"

#include "../include/RapidJson/rapidjson.h"
#include "../include/rapidjson/encodings.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/filereadstream.h"
using namespace rapidjson;

#include "ProcessExcutor.h"

#pragma warning(disable:4996)

#ifdef _DEBUG
#pragma comment(lib, "../Debug/zlib.lib")
#else
#pragma comment(lib, "../Release/zlib.lib")
#endif

# pragma comment(lib, "wbemuuid.lib")

typedef struct
{
	__int64	tLastModified;
	__int64	nContentSize;
	wchar_t szMD5[68];
	DWORD	offsetContent;
	DWORD	offsetNextFile;
}FileIndex;

class CInstallUtil
{
public:
	CInstallUtil()
	{
		m_aryRecyleList.clear();

		m_strInstallFolder = "";

		m_Globals.clear();
		m_OnInit.clear();

		m_Logger = NULL;
	}
	~CInstallUtil()
	{
		Cleanup();
	}

public:
	static CString GetComputerName()
	{
		TCHAR  infoBuf[512];
		DWORD bufCharCount = 512;

		if (::GetComputerNameW(infoBuf, &bufCharCount))
			return infoBuf;

		return L"";
	}

	static CString GetUserName()
	{
		TCHAR  infoBuf[512];
		DWORD bufCharCount = 512;

		if (::GetUserNameW(infoBuf, &bufCharCount))
			return infoBuf;

		return L"";
	}

	static CString GetSystemDirectory()
	{
		TCHAR  infoBuf[512];
		if (::GetSystemDirectoryW(infoBuf, 512))
			return infoBuf;

		return L"";
	}

	static CString GetSystemDrive()
	{
		CString strSystem = GetSystemDirectory();
		if (strSystem.IsEmpty())
			return L"C:";
		return strSystem.Left(2);
	}

	static CString GetWindowsDirectory()
	{
		TCHAR  infoBuf[512];
		if (::GetWindowsDirectoryW(infoBuf, 512))
			return infoBuf;

		return L"";
	}

	static BOOL ScanFolder(const wchar_t* szFolder, CStringArray& aryResultList)
	{
		CString strFolder = szFolder;
		strFolder.Replace(L"/", L"\\");
		if (strFolder.Right(1) != L"\\")
			strFolder += L"\\";

		CString strFilter = strFolder + L"*.*";

		long	hFile = -1;
		BOOL	bFirst = TRUE;
		struct	_wfinddata_t c_file;

		CStringArray arySubFolders;
		while (TRUE)
		{
			if (bFirst)
			{
				bFirst = FALSE;
				if ((hFile = _wfindfirst(strFilter, &c_file)) == -1L)
					return FALSE;
			}
			else
			{
				if (_wfindnext(hFile, &c_file) == -1L)
					break;
			}

			if (c_file.attrib & _A_RDONLY)
				continue;
			if (c_file.attrib & _A_SYSTEM)
				continue;
			if (c_file.attrib & _A_HIDDEN)
				continue;
			if (c_file.attrib & _A_SUBDIR)
			{
				if ((wcscmp(c_file.name, L".") != 0) && (wcscmp(c_file.name, L"..") != 0))
					arySubFolders.Add(strFolder + c_file.name + L"\\");
				continue;
			}
			aryResultList.Add(strFolder + c_file.name);
		}

		for (int i = 0; i < arySubFolders.GetCount(); i++)
		{
			ScanFolder(arySubFolders[i], aryResultList);
		}

		if (hFile > 0)
			_findclose(hFile);

		return TRUE;
	}

	static int GetNativeSystemProcessorArchitecture()
	{
		SYSTEM_INFO si;
		memset(&si, 0, sizeof(si));
		GetNativeSystemInfo(&si);

		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
		case PROCESSOR_ARCHITECTURE_IA64:
		case PROCESSOR_ARCHITECTURE_ARM64:
			return 64;
		case PROCESSOR_ARCHITECTURE_INTEL:
			return 32;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			break;
		}
		return 0;
	}

	static BOOL DirectoryExists(const wchar_t* dirName)
	{
		DWORD attribs = ::GetFileAttributesW(dirName);
		if (attribs == INVALID_FILE_ATTRIBUTES)
			return false;
		return (attribs & FILE_ATTRIBUTE_DIRECTORY);
	}

	static CString GetCodeBase()
	{
		CString strRunPath = L"";
		wchar_t	szIniPath[MAX_PATH];

		DWORD dwResult = GetModuleFileNameW(NULL, szIniPath, sizeof(szIniPath));
		if (0 != dwResult)
		{
			strRunPath = szIniPath;
			strRunPath.Replace(L"/", L"\\");
			int index = strRunPath.ReverseFind(L'\\');
			if (index != -1)
				strRunPath = strRunPath.Left(index + 1);
		}
		return strRunPath;
	}

	static CString GetCodePath()
	{
		CString strRunPath = L"";
		wchar_t	szIniPath[MAX_PATH];

		DWORD dwResult = GetModuleFileNameW(NULL, szIniPath, sizeof(szIniPath));
		if (0 != dwResult)
		{
			strRunPath = szIniPath;
			strRunPath.Replace(L"/", L"\\");
		}
		return strRunPath;
	}

	static CString GetCmdLine()
	{
		CString strCommandLine = L"";
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
		if (NULL != szArglist)
		{
			for (int i = 0; i < nArgs; i++)
			{
				if (i != 0)
					strCommandLine += L" ";
				strCommandLine += szArglist[i];
			}
		}
		LocalFree(szArglist);
		return strCommandLine;
	}

	static void CreateDirectoryForPath(CString strPath)
	{
		wchar_t* sPtr1, * sPtr2, cTmp;
		wchar_t	TmpString[_MAX_PATH];

		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];

		if (_waccess(strPath, 0) > 0)
			return;

		_wsplitpath(strPath, drive, dir, fname, ext);

		CString strTemp = strPath;
		strTemp.Replace(L"/", L"\\");
		wcscpy(TmpString, strTemp);

		sPtr1 = TmpString;
		while (TRUE)
		{
			sPtr2 = wcschr(sPtr1, L'\\');
			if (sPtr2 == NULL)
				break;

			cTmp = sPtr2[0];
			sPtr2[0] = 0;
			_wmkdir(TmpString);

			sPtr2[0] = cTmp;
			sPtr1 = sPtr2 + 1;
		}
		Sleep(100);
	}

	static BOOL IsPathExisted(CString strDir)
	{
		long	hFile;
		struct	_wfinddata_t c_file;

		if ((hFile = _wfindfirst(strDir, &c_file)) == -1L)
			return FALSE;
		_findclose(hFile);

		return TRUE;
	}

	static BOOL DeleteDirectory(const wchar_t* sPath, BOOL bOnlyRemoveEmptyFolder) 
	{
		HANDLE hFind;
		WIN32_FIND_DATAW FindFileData;
		CString strPath = sPath;
		if (strPath.Right(1) != L"\\")
			strPath += L"\\";

		wchar_t DirPath[MAX_PATH];
		_tcscpy(DirPath, strPath);
		_tcscat(DirPath, L"*");

		wchar_t FileName[MAX_PATH];
		_tcscpy(FileName, strPath);

		hFind = FindFirstFileW(DirPath, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) 
			return FALSE;
		_tcscpy(DirPath, FileName);

		bool bSearch = true;
		while (bSearch) 
		{
			if (FindNextFileW(hFind, &FindFileData)) 
			{
				if ((_tcscmp(FindFileData.cFileName, L".") == 0) ||
					(_tcscmp(FindFileData.cFileName, L"..") == 0))
				{
					continue;
				}
				_tcscat(FileName, FindFileData.cFileName);
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
				{
					try
					{
						DeleteDirectory(FileName, bOnlyRemoveEmptyFolder);
						RemoveDirectoryW(FileName);
					}
					catch (CException* e)
					{
						e->Delete();
					}
					_tcscpy(FileName, DirPath);
				}
				else 
				{
					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						_wchmod(FileName, _S_IWRITE);

					if (!bOnlyRemoveEmptyFolder)
					{
						try
						{
							DeleteFileW(FileName);
						}
						catch (CException* e)
						{
							e->Delete();
						}
					}
					_tcscpy(FileName, DirPath);
				}
			}
			else 
			{
				if (GetLastError() == ERROR_NO_MORE_FILES)
					bSearch = false;
				else 
				{
					FindClose(hFind);
					return FALSE;
				}
			}
		}
		FindClose(hFind);

		BOOL bRemoveResult = FALSE;
		try
		{
			bRemoveResult = RemoveDirectoryW(sPath);
		}
		catch (CException* e)
		{
			e->Delete();
		}

		return bRemoveResult;
	}

	static BOOL DeleteDirectory(CString strRootDir, BOOL bEmptyOnly, BOOL bDeleteRoot)
	{
		CFile	file;
		long	hFile;
		BOOL	bFirst = TRUE;
		struct	_wfinddata_t c_file;
		CString strMatchPath;

		CString strDir = strRootDir;
		strDir.Replace(L"/", L"\\");

		if (strDir.Right(1) != L"\\")
			strDir += L"\\";

		strMatchPath = strDir + L"*.*";

		for (int iItem = 0; ;)  // will now insert the items and subitems into the list view.
		{
			if (bFirst)
			{
				bFirst = FALSE;
				if ((hFile = _wfindfirst(strMatchPath, &c_file)) == -1L)
					return FALSE;
			}
			else
			{
				if (_wfindnext(hFile, &c_file) == -1L)
					break;
			}

			if (c_file.attrib & _A_SUBDIR)
			{
				if ((wcscmp(c_file.name, L".") == 0) ||
					(wcscmp(c_file.name, L"..") == 0))
				{
					continue;
				}
				else
				{
					CString strNDir = strDir + c_file.name;
					DeleteDirectory(strNDir, bEmptyOnly, TRUE);
				}
			}
			else if (!bEmptyOnly)
				_wunlink(strDir + c_file.name);
		}
		_findclose(hFile);

		try
		{
			if (bDeleteRoot)
			{
				CString strDeleteDir = strDir.Left(strDir.GetLength() - 1);
				_wrmdir(strDeleteDir);
			}
		}
		catch (...)
		{
		}

		Sleep(100);
		return TRUE;
	}

	static int SplitString(CStringArray& result, CString szString, CString szTokens, BOOL bAsOneToken = FALSE)
	{
		int iPos = 0;
		CString subString;
		CString strLeft = szString;

		result.RemoveAll();
		result.FreeExtra();

		int nDelimiterLen = 1;
		if (bAsOneToken)
			nDelimiterLen = wcslen(szTokens);

		while (TRUE)
		{
			if (strLeft.IsEmpty())
			{
				result.Add(strLeft);
				break;
			}

			if (bAsOneToken)
				iPos = strLeft.Find(szTokens);
			else
				iPos = strLeft.FindOneOf(szTokens);

			if (iPos < 0)
			{
				result.Add(strLeft);
				break;
			}

			subString = strLeft.Left(iPos);
			result.Add(subString);

			strLeft = strLeft.Mid(iPos + nDelimiterLen);
		}

		return result.GetSize();
	}

	static BOOL ParseKeyValuePair(const wchar_t* strParamList, const wchar_t* strItemSpe, const wchar_t* strPairSpe, map<CString, CString>& mapJobAttribs)
	{
		CStringArray aryJobAttribs;
		CStringArray aryAttribPair;

		if (!SplitString(aryJobAttribs, strParamList, strItemSpe, TRUE))
			return FALSE;

		mapJobAttribs.clear();
		for (int iCol = 0; iCol < aryJobAttribs.GetSize(); iCol++)
		{
			if (!SplitString(aryAttribPair, aryJobAttribs[iCol], strPairSpe, TRUE))
				continue;
			if (aryAttribPair.GetSize() != 2)
				continue;
			mapJobAttribs[aryAttribPair[0]] = aryAttribPair[1];
		}

		return (mapJobAttribs.size() > 0);
	}

	static void PackingFiles(CStringArray& aryFileList, int nRelPathOffset, const wchar_t* szPackFile)
	{
		CFile outFile;
		FileIndex fIndex;

		__int64 nWritePos = 0;
		CFileStatus status;

		int nCacheSize = 16384;
		char* pBuffer = new char[nCacheSize];
		if (pBuffer != NULL)
		{
			outFile.Open(szPackFile, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary, NULL);
			for (int i = 0; i < aryFileList.GetCount(); i++)
			{
				if (CFile::GetStatus(aryFileList[i], status))
				{
					wchar_t szRelPath[MAX_PATH];
					memset(szRelPath, 0, sizeof(szRelPath));

					CString relPath = aryFileList[i].Mid(nRelPathOffset);
					wcscpy(szRelPath, relPath);

					CString md5 = L"";
					if (status.m_size > 0)
						md5 = GetMD5(aryFileList[i]);

					int nRelPathLen = wcslen(szRelPath) * sizeof(wchar_t) + 2;
					nRelPathLen = ((nRelPathLen + 7) >> 3) << 3; //2-byte NULL, 8-byte alignment

					memset(&fIndex, 0, sizeof(fIndex));
					wcscpy(fIndex.szMD5, md5);
					fIndex.tLastModified = status.m_mtime.GetTime();
					fIndex.nContentSize = status.m_size; //content
					fIndex.offsetContent = sizeof(FileIndex) + nRelPathLen; //index+path
					fIndex.offsetNextFile = (DWORD)(nWritePos + sizeof(FileIndex) + nRelPathLen + status.m_size); //index+path+content

					if (outFile.m_hFile != CFile::hFileNull)
					{
						outFile.Write(&fIndex, sizeof(fIndex));
						nWritePos += sizeof(fIndex);

						memset(pBuffer, 0, nCacheSize);
						wcscpy((wchar_t*)pBuffer, szRelPath);
						outFile.Write(pBuffer, nRelPathLen);
						nWritePos += nRelPathLen;

						CFile inFile;
						if (inFile.Open(aryFileList[i], CFile::modeRead | CFile::typeBinary, NULL))
						{
							while (status.m_size > 0)
							{
								int nRead = inFile.Read(pBuffer, nCacheSize);
								if (nRead <= 0)
									break;
								outFile.Write(pBuffer, nRead);
								nWritePos += nRead;
							}
							inFile.Close();
						}
					}
				}
			}
			outFile.Close();
			delete[] pBuffer;
		}
	}

	static CString GetTempFile(CString strPrefix, CString strExt)
	{
		CString strTempFile = L"";

		wchar_t lpTempPathBuffer[MAX_PATH];
		DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
		if (dwRetVal > MAX_PATH || (dwRetVal == 0))
			return strTempFile;

		wchar_t szTempFileName[MAX_PATH];
		UINT uRetVal = GetTempFileName(lpTempPathBuffer, strPrefix, 0, szTempFileName);
		if (uRetVal == 0)
			return strTempFile;

		strTempFile = szTempFileName;
		if (!strExt.IsEmpty())
		{
			strTempFile.Replace(L".tmp", L"");
			if (strExt.Left(1) != L".")
				strTempFile += L".";
			strTempFile += strExt;
		}
		return strTempFile;
	}

	static BOOL CompressFile(CString src, CString dst)
	{
		BOOL bSuccess = FALSE;

		CFile fSrc, fDst;

		fSrc.Open(src, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, NULL);
		if (fSrc.m_hFile != CFile::hFileNull)
		{
			long nSrcSize = (long)fSrc.GetLength();
			BYTE* pSrcBuf = new BYTE[nSrcSize];
			if (pSrcBuf)
			{
				fSrc.Read(pSrcBuf, nSrcSize);

				int nSizeDst = GetMaxCompressedLen(nSrcSize + 2);
				BYTE* pDstBuf = new BYTE[nSizeDst];
				if (pDstBuf)
				{
					z_stream zInfo = { 0 };
					zInfo.total_in = zInfo.avail_in = nSrcSize;
					zInfo.total_out = zInfo.avail_out = nSizeDst;
					zInfo.next_in = pSrcBuf;
					zInfo.next_out = pDstBuf;

					int nRet = -1;
					int nErr = deflateInit(&zInfo, Z_DEFAULT_COMPRESSION); // zlib function
					if (nErr == Z_OK)
					{
						nErr = deflate(&zInfo, Z_FINISH);              // zlib function
						if (nErr == Z_STREAM_END)
							nRet = zInfo.total_out;
					}
					deflateEnd(&zInfo);    // zlib function

					if (nRet > 0)
					{
						fDst.Open(dst, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyNone, NULL);
						if (fSrc.m_hFile != CFile::hFileNull)
						{
							fDst.Write(pDstBuf, nRet);
							fDst.Close();

							bSuccess = TRUE;
						}
					}
					delete[] pDstBuf;
				}
				delete[] pSrcBuf;
			}
			fSrc.Close();
		}

		return bSuccess;
	}

	static int UncompressFile(CString src, CString dst)
	{
		BOOL bSuccess = FALSE;

		CFile fSrc, fDst;

		fSrc.Open(src, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, NULL);
		if (fSrc.m_hFile != CFile::hFileNull)
		{
			long nSrcSize = (long)fSrc.GetLength();
			BYTE* pSrcBuf = new BYTE[nSrcSize];
			if (pSrcBuf)
			{
				fSrc.Read(pSrcBuf, nSrcSize);

				int nSizeDst = 10 * nSrcSize;
				if (nSizeDst > 0)
				{
					BYTE* pDstBuf = new BYTE[nSizeDst];
					if (pDstBuf)
					{
						z_stream zInfo = { 0 };

						zInfo.total_in = zInfo.avail_in = nSrcSize;
						zInfo.total_out = zInfo.avail_out = nSizeDst;
						zInfo.next_in = pSrcBuf;
						zInfo.next_out = pDstBuf;

						int nRet = -1;
						int nErr = inflateInit(&zInfo);               // zlib function
						if (nErr == Z_OK)
						{
							nErr = inflate(&zInfo, Z_FINISH);     // zlib function
							if (nErr == Z_STREAM_END)
							{
								nRet = zInfo.total_out; // -1 or len of output
							}
						}
						inflateEnd(&zInfo);   // zlib function

						if (nRet > 0)
						{
							fDst.Open(dst, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyNone, NULL);
							if (fSrc.m_hFile != CFile::hFileNull)
							{
								fDst.Write(pDstBuf, nRet);
								fDst.Close();

								bSuccess = TRUE;
							}
						}
						delete[] pDstBuf;
					}
				}
				delete[] pSrcBuf;
			}
			fSrc.Close();
		}

		return bSuccess;
	}

	static BOOL ExtractBinResource(const wchar_t* szResourceName, int nResourceId, const wchar_t* szOutputFile)
	{
		HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(nResourceId), szResourceName);
		if (hRes == NULL)
			return FALSE;

		HGLOBAL hResource = LoadResource(NULL, hRes);
		if (hResource == NULL)
		{
			return FALSE;
		}

		char* lpResLock = (char*)LockResource(hResource);
		if (lpResLock == NULL)
		{
			return FALSE;
		}

		DWORD dwSizeRes = SizeofResource(NULL, hRes);

		std::ofstream outputFile(szOutputFile, std::ios::binary);

		outputFile.write((const char*)lpResLock, dwSizeRes);
		outputFile.close();

		return TRUE;
	}

	static CString GetMD5(CString strFile)
	{
#define BUFSIZE 1024
#define MD5LEN  16

		DWORD dwStatus = 0;
		BOOL bResult = FALSE;

		HCRYPTPROV hProv = 0;
		HCRYPTHASH hHash = 0;

		BYTE rgbFile[BUFSIZE];
		DWORD cbRead = 0;

		BYTE rgbHash[MD5LEN];
		DWORD cbHash = 0;

		HANDLE hFile = CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			dwStatus = GetLastError();
			TRACE("Error opening file %s\nError: %d\n", strFile, dwStatus);
			return L"";
		}

		DWORD dwHiSize;
		if (GetFileSize(hFile, &dwHiSize) <= 0)
		{
			CloseHandle(hFile);
			return L"";
		}

		// Get handle to the crypto provider
		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		{
			dwStatus = GetLastError();
			TRACE("CryptAcquireContext failed: %d\n", dwStatus);
			CloseHandle(hFile);
			return L"";
		}

		if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
			dwStatus = GetLastError();
			TRACE("CryptAcquireContext failed: %d\n", dwStatus);
			CloseHandle(hFile);
			CryptReleaseContext(hProv, 0);
			return L"";
		}

		while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL))
		{
			if (0 == cbRead)
				break;

			if (!CryptHashData(hHash, rgbFile, cbRead, 0))
			{
				dwStatus = GetLastError();
				TRACE("CryptHashData failed: %d\n", dwStatus);
				CryptReleaseContext(hProv, 0);
				CryptDestroyHash(hHash);
				CloseHandle(hFile);
				return L"";
			}
		}

		if (!bResult)
		{
			dwStatus = GetLastError();
			TRACE("ReadFile failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return L"";
		}

		CString strMD5 = L"";
		cbHash = MD5LEN;
		if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
		{
			CHAR rgbDigits[] = "0123456789abcdef";
			for (DWORD i = 0; i < cbHash; i++)
			{
				CString strTemp;
				strTemp.Format(L"%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);
				strMD5 += strTemp;
			}
		}
		else
		{
			dwStatus = GetLastError();
			TRACE("CryptGetHashParam failed: %d\n", dwStatus);
		}

		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		CloseHandle(hFile);

		strMD5.MakeUpper();
		return strMD5;
	}

	static CString GetProcessNameByID(DWORD processID)
	{
		WCHAR szProcessName[MAX_PATH] = L"";
		HMODULE hMod;
		DWORD cbNeeded;
		CString strProcessName = L"";

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		if (hProcess != NULL)
		{
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			{
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(WCHAR));
				strProcessName = szProcessName;

				MODULEINFO modelInfo;
				if (GetModuleInformation(hProcess, hMod, &modelInfo, sizeof(modelInfo)))
				{
					int debug = 0;
				}
			}
		}
		CloseHandle(hProcess);
		return strProcessName;
	}

	static BOOL ListProcessModules(DWORD dwPID, CString& strModuleName, CString& exePath)
	{
		HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
		MODULEENTRY32 me32;

		strModuleName = "";
		exePath = "";

		// Take a snapshot of all modules in the specified process.
		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
		if (hModuleSnap == INVALID_HANDLE_VALUE)
		{
			TRACE(TEXT("CreateToolhelp32Snapshot (of modules)"));
			return(FALSE);
		}

		// Set the size of the structure before using it.
		me32.dwSize = sizeof(MODULEENTRY32);

		// Retrieve information about the first module,
		// and exit if unsuccessful
		if (!Module32First(hModuleSnap, &me32))
		{
			TRACE(TEXT("Module32First"));  // show cause of failure
			CloseHandle(hModuleSnap);      // clean the snapshot object
			return(FALSE);
		}

		// Now walk the module list of the process,
		// and display information about each module
		do
		{
			strModuleName = me32.szModule;
			exePath = me32.szExePath;
			if (!exePath.IsEmpty())
				break;

			TRACE(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
			TRACE(TEXT("\n     Executable     = %s"), me32.szExePath);
			TRACE(TEXT("\n     Process ID     = 0x%08X"), me32.th32ProcessID);
			TRACE(TEXT("\n     Ref count (g)  = 0x%04X"), me32.GlblcntUsage);
			TRACE(TEXT("\n     Ref count (p)  = 0x%04X"), me32.ProccntUsage);
			TRACE(TEXT("\n     Base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
			TRACE(TEXT("\n     Base size      = %d"), me32.modBaseSize);
		} while (Module32Next(hModuleSnap, &me32));

		CloseHandle(hModuleSnap);
		return(TRUE);
	}

	static map<DWORD, map<CString, CString>> GetProcessInfo(void)
	{
		map<DWORD, map<CString, CString>> proceses;
		DWORD aProcesses[1024], cbNeeded, cProcesses;

		unsigned int i;
		if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			// Calculate how many process identifiers were returned.
			cProcesses = cbNeeded / sizeof(DWORD);
			// Print the name and process identifier for each process.
			for (i = 0; i < cProcesses; i++)
			{
				if (aProcesses[i] != 0)
				{
					CString strModuleName, exePath;
					ListProcessModules(aProcesses[i], strModuleName, exePath);

					//CString strProcessName = GetProcessNameByID(aProcesses[i]);
					if (!strModuleName.IsEmpty())
					{
						map<CString, CString> processInfo;
						processInfo[L"CommandLine"] = exePath;
						processInfo[L"ExecutablePath"] = exePath;

						proceses[aProcesses[i]] = processInfo;
					}
				}
			}
		}
		return proceses;
	}
/*
	static map<DWORD, map<CString, CString>> GetProcessInfo(void)
	{
		map<DWORD, map<CString, CString>> proceses;

		HRESULT hr = 0;
		CString strLog;
		IWbemLocator* WbemLocator = NULL;
		IWbemServices* WbemServices = NULL;
		IEnumWbemClassObject* EnumWbem = NULL;

		hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			strLog.Format(L"CoInitialize %08lX", hr);
			OutputDebugString(strLog);
			return proceses;
		}

		hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
		if (FAILED(hr))
		{
			CoUninitialize();
			strLog.Format(L"CoInitializeSecurity %08lX", hr);
			OutputDebugString(strLog);
			return proceses;
		}

		GUID CLSID_WbemLocator = { 0x4590F811,0x1D3A,0x11D0,{ 0x89,0x1F,0x00,0xAA,0x00,0x4B,0x2E,0x24 } };
		hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&WbemLocator);
		if (FAILED(hr) || (WbemLocator == NULL))
		{
			CoUninitialize();
			strLog.Format(L"CoCreateInstance %08lX", hr);
			OutputDebugString(strLog);
			return proceses;
		}

		hr = WbemLocator->ConnectServer(L"ROOT\\CIMV2", NULL, NULL, NULL, 0, NULL, NULL, &WbemServices);
		if (FAILED(hr) || (WbemServices == NULL))
		{
			WbemLocator->Release();

			CoUninitialize();
			strLog.Format(L"ConnectServer %08lX", hr);
			OutputDebugString(strLog);
			return proceses;
		}

		hr = WbemServices->ExecQuery(L"WQL", L"SELECT ProcessId,CommandLine,ExecutablePath FROM Win32_Process", WBEM_FLAG_FORWARD_ONLY, NULL, &EnumWbem);
		if (FAILED(hr) || (EnumWbem == NULL))
		{
			WbemServices->Release();
			WbemLocator->Release();

			CoUninitialize();
			strLog.Format(L"ExecQuery %08lX", hr);
			OutputDebugString(strLog);
			return proceses;
		}

		IWbemClassObject* result = NULL;
		ULONG returnedCount = 0;

		while ((hr = EnumWbem->Next(WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK)
		{
			VARIANT ProcessId;
			VARIANT CommandLine;
			VARIANT ExecutablePath;

			hr = result->Get(L"ProcessId", 0, &ProcessId, 0, 0);
			if (SUCCEEDED(hr))
			{
				map<CString, CString> processInfo;
				hr = result->Get(L"CommandLine", 0, &CommandLine, 0, 0);
				if (SUCCEEDED(hr))
				{
					processInfo[L"CommandLine"] = CommandLine.bstrVal;

					hr = result->Get(L"ExecutablePath", 0, &ExecutablePath, 0, 0);
					if (SUCCEEDED(hr))
					{
						processInfo[L"ExecutablePath"] = ExecutablePath.bstrVal;
					}
				}
				proceses[ProcessId.uintVal] = processInfo;
			}
			result->Release();
		}

		EnumWbem->Release();
		WbemServices->Release();
		WbemLocator->Release();

		CoUninitialize();

		return proceses;
	}
*/
	/*
	static vector<CString> GetProcessModules(DWORD processID)
	{
		vector<CString> modules;

		HMODULE hMods[2048];
		HANDLE hProcess;
		DWORD cbNeeded;
		unsigned int i;

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		if (NULL == hProcess)
			return modules;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				TCHAR szProcessName[MAX_PATH];
				TCHAR szModName[MAX_PATH];
				if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					GetModuleBaseName(hProcess, hMods[i], szProcessName, sizeof(szProcessName) / sizeof(WCHAR));
					//modules.push_back(szProcessName);
					modules.push_back(szModName);
				}
			}
		}

		CloseHandle(hProcess);

		return modules;
	}

	static map<DWORD, vector<CString>> GetProcessModules(void)
	{
		map<DWORD, vector<CString>> processes;

		DWORD aProcesses[2048];
		DWORD cbNeeded;
		DWORD cProcesses;
		unsigned int i;

		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
			return processes;

		cProcesses = cbNeeded / sizeof(DWORD);

		for (i = 0; i < cProcesses; i++)
		{
			vector<CString> modules = GetProcessModules(aProcesses[i]);
			processes[aProcesses[i]] = modules;
		}
		return processes;
	}
	*/
	//https://stackoverflow.com/questions/33841911/c-how-do-i-create-a-shortcut-in-the-start-menu-on-windows
	static BOOL CreateLink(const wchar_t* lpszLinkName, const wchar_t* lpszLinkPath, const wchar_t* lpszTargetPath, const wchar_t* lpszTargetFolder, const wchar_t* lpszIconPath)
	{
		BOOL bResult = FALSE;

		IShellLink* pShellLink = NULL;
		HRESULT hRes = CoInitialize(NULL);

		if (!SUCCEEDED(hRes))
			return bResult;

		hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
		if (SUCCEEDED(hRes))
		{
			IPersistFile* pPersistFile;

			pShellLink->SetPath(lpszTargetPath);
			pShellLink->SetDescription(lpszLinkName);
			pShellLink->SetWorkingDirectory(lpszTargetFolder);

			if (wcslen(lpszIconPath) > 0)
				pShellLink->SetIconLocation(lpszIconPath, 0);
			else
				pShellLink->SetIconLocation(lpszTargetPath, 0);

			hRes = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
			if (SUCCEEDED(hRes))
			{
				CreateDirectoryForPath(lpszLinkPath);

				hRes = pPersistFile->Save(lpszLinkPath, TRUE);
				if (SUCCEEDED(hRes))
					bResult = TRUE;

				pPersistFile->Release();
			}
			pShellLink->Release();
		}

		CoUninitialize();

		return bResult;
	}

	static CString FormatPath(CString path, char chCaseA, int nEndSeparator)
	{
		CString strFormated = path;

		if (chCaseA == L'a')
			strFormated.MakeLower();
		else if (chCaseA == L'A')
			strFormated.MakeUpper();

		strFormated.Trim();
		strFormated.Replace(L"/", L"\\");

		if ((nEndSeparator > 0) && (strFormated.Right(1) != L"\\"))
			strFormated += L"\\";
		if ((nEndSeparator < 0) && (strFormated.Right(1) == L"\\"))
			strFormated = strFormated.Left(strFormated.GetLength() - 1);

		return strFormated;
	}

	static CString GetFolder(CString path)
	{
		CString strPath = FormatPath(path, 0, 0);
		int idx = strPath.ReverseFind(L'\\');
		if (idx >= 0)
			return strPath.Left(idx);
		return strPath;
	}

	static BOOL IsRunning(CString strAppPath)
	{
		CString path = FormatPath(strAppPath, L'a', 0);

		map<DWORD, map<CString, CString>> all = GetProcessInfo();
		map<DWORD, map<CString, CString>>::iterator iter;
		for (iter = all.begin(); iter != all.end(); iter++)
		{
			CString strRunning = iter->second[L"ExecutablePath"]; //CommandLine
			strRunning = FormatPath(strRunning, L'a', 0);
			if (strRunning == path)
				return TRUE;
		}
		return FALSE;
	}

	static BOOL IsRunAsAdmin()
	{
		BOOL fIsRunAsAdmin = FALSE;
		DWORD dwError = ERROR_SUCCESS;
		PSID pAdministratorsGroup = NULL;

		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
		{
			if (CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
			{
			}
		}

		if (pAdministratorsGroup) 
			FreeSid(pAdministratorsGroup);

		return fIsRunAsAdmin;
	}

	static BOOL ExecuteCommandLine(const wchar_t* cmdLine, DWORD dwMilliSeconds, BOOL bUseCmdFolder, BOOL bShow)
	{
		DWORD exitCode = 0;

		SECURITY_ATTRIBUTES   saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.lpSecurityDescriptor = NULL;
		saAttr.bInheritHandle = TRUE;

		PROCESS_INFORMATION processInformation = { 0 };
		STARTUPINFOW startupInfo = { 0 };
		startupInfo.cb = sizeof(startupInfo);

		DWORD dwCreateFlag = NORMAL_PRIORITY_CLASS;
		if (!bShow)
			dwCreateFlag |= CREATE_NO_WINDOW;

		wchar_t szFolder[MAX_PATH];
		memset(szFolder, 0, sizeof(szFolder));

		BOOL result = FALSE;
		if (bUseCmdFolder)
		{
			wcscpy(szFolder, GetFolder(cmdLine));
			result = CreateProcessW(NULL, (wchar_t*)cmdLine, &saAttr, &saAttr, TRUE, dwCreateFlag, NULL, szFolder, &startupInfo, &processInformation);
		}
		else
		{
			result = CreateProcessW(NULL, (wchar_t*)cmdLine, &saAttr, &saAttr, TRUE, dwCreateFlag, NULL, NULL, &startupInfo, &processInformation);
		}
		if (!result)
		{
			exitCode = 0;
			return FALSE;
		}
		else
		{
			WaitForSingleObject(processInformation.hProcess, dwMilliSeconds);

			result = GetExitCodeProcess(processInformation.hProcess, &exitCode);

			CloseHandle(processInformation.hProcess);
			CloseHandle(processInformation.hThread);

			if (!result)
				return FALSE;

			return TRUE;
		}
	}

	static int GetMaxCompressedLen(int nLenSrc)
	{
		int n16kBlocks = (nLenSrc + 16383) / 16384; // round up any fraction of a block
		return (nLenSrc + 6 + (n16kBlocks * 5));
	}

	static BOOL GetJsonDocument(CString jsonFile, rapidjson::Document& document)
	{
		FILE* fp = _wfopen(jsonFile, L"rb");
		if (NULL == fp)
			return FALSE;

		fseek(fp, 0L, SEEK_END);
		int length = ftell(fp) + 1;

		char* buff = new char[length];
		if (NULL == buff)
		{
			fclose(fp);
			return FALSE;
		}
		memset(buff, 0, length);
		fseek(fp, 0L, SEEK_SET);

		rapidjson::FileReadStream inputStream(fp, buff, length);
		document.ParseStream(inputStream);

		delete[]buff;
		buff = NULL;

		fclose(fp);
		return TRUE;
	}

	static map<CString, CString> GetRootInfo(CString jsonFile)
	{
		map<CString, CString> result;

		rapidjson::Document document;
		if (!GetJsonDocument(jsonFile, document))
			return result;

		map<CString, CString> action;
		for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
		{
			if (itr->value.IsString())
			{
				CString strKey = CString(itr->name.GetString());
				strKey.MakeLower();
				result[strKey] = itr->value.GetString();
			}
		}

		return result;
	}

	static BOOL GetSection(CString jsonFile, const char* szSection, vector<map<CString, CString>>& result)
	{
		result.clear();

		rapidjson::Document document;
		if (!GetJsonDocument(jsonFile, document))
			return FALSE;

		if (document.HasMember(szSection))
		{
			rapidjson::Value& obj = document[szSection];
			if (obj.IsArray())
			{
				for (size_t i = 0; i < obj.Size(); i++)
				{
					if (!obj[i].IsObject())
						continue;
					if (!obj[i].HasMember("type"))
						continue;

					map<CString, CString> action;
					for (Value::ConstMemberIterator itr = obj[i].MemberBegin(); itr != obj[i].MemberEnd(); ++itr)
					{
						CString strValue(itr->value.GetString());

						CStringArray aryTags;
						SplitString(aryTags, strValue, L"%", TRUE);

						for (int j = 0; j < aryTags.GetCount(); j++)
						{
							if ((j & 1) > 0)
							{
								CString lowerTag = aryTags[j];
								lowerTag.MakeLower();

								strValue.Replace(L"%" + aryTags[j] + L"%", L"%" + lowerTag + L"%"); //make tag name lowercase
							}
						}
						action[CString(itr->name.GetString())] = strValue;
					}
					result.push_back(action);
				}
			}
		}
		return TRUE;
	}

	static BOOL GetOnInit(CString jsonFile, vector<map<CString, CString>>& result)
	{
		return GetSection(jsonFile, "on_init", result);
	}

	static BOOL GetOnInstall(CString jsonFile, vector<map<CString, CString>>& result)
	{
		return GetSection(jsonFile, "on_install", result);
	}

	static BOOL GetOnUninstall(CString jsonFile, vector<map<CString, CString>>& result)
	{
		return GetSection(jsonFile, "on_uninstall", result);
	}

	static map<CString, map<CString, CString>> ScanInstalledFiles(CString strCodebase)
	{
		map<CString, map<CString, CString>> installedFiles;

		CString strSetupList;
		strSetupList.Format(L"%s.setup\\.setup", strCodebase);

		if (_waccess(strSetupList, 0) != -1)
		{
			CStdioFile fSetup;
			fSetup.Open(strSetupList, CFile::modeRead | CFile::typeUnicode, NULL);
			if (fSetup.m_hFile != CFile::hFileNull)
			{
				while (TRUE)
				{
					CString strFilePath;
					if (!fSetup.ReadString(strFilePath))
						break;

					map<CString, CString> checks;
					int idx = strFilePath.Find(L",");
					if (idx >= 0)
					{
						CString md5Setup = strFilePath.Mid(idx + 1);
						md5Setup.Replace(L"\r", L"");
						md5Setup.Replace(L"\n", L"");

						checks[L"setupMD5"] = md5Setup;
						checks[L"MD5"] = GetMD5(strFilePath.Left(idx));

						installedFiles[strFilePath.Left(idx)] = checks;
					}
				}
				fSetup.Close();
			}
		}
		return installedFiles;
	}

	static map<CString, CString> getWellknownTags() //key is lowercase
	{
		/*
		%SystemDrive% : C:
		%SystemRoot% : %SystemDrive%\Windows\System32
		%windir% :     %SystemDrive%\WINDOWS
		%ProgramFiles% : %SystemDrive%\Program Files
		%ProgramFiles(x86)% : %SystemDrive%\Program Files (x86)

		%ALLUSERSPROFILE% : C:\ProgramData
		%APPDATA% : C:\Users\UserName\AppData\Roaming
		%CommonProgramFiles% : C:\Program Files\Common Files

		%CommonProgramFiles(x86)% : C:\Program Files (x86)\Common Files
		%CommonProgramW6432% : C:\Program Files\Common Files
		%COMPUTERNAME% : ComputerName
		%ComSpec% : C:\Windows\System32\cmd.exe
		%HOMEDRIVE% : C:
		%HOMEPATH% : \Users\UserName

		%LOCALAPPDATA% : C:\Users\UserName\AppData\Local
		%ProgramData% : %SystemDrive%\ProgramData
		%ProgramW6432% : %SystemDrive%\Program Files
		%USERNAME% :
		*/

		map<CString, CString> tags;

		CString strSysDrive = GetSystemDrive();
		tags[L"systemdrive"] = strSysDrive;
		tags[L"systemroot"] = strSysDrive + L"\\Windows";
		tags[L"windir"] = strSysDrive + L"\\Windows";
		tags[L"programfiles"] = strSysDrive + L"\\Program Files";
		tags[L"programfiles(x86)"] = strSysDrive + L"\\Program Files (x86)";
		tags[L"programdata"] = strSysDrive + L"\\ProgramData";

		return tags;
	}

private:
	vector<CString> m_aryRecyleList;

	CString m_strInstallFolder;

	map<CString, CString> m_Globals;
	map<CString, CString> m_LogFormats;

	vector<map<CString, CString>> m_OnInit;
	vector<map<CString, CString>> m_OnInstall;
	vector<map<CString, CString>> m_OnUninstall;

	void (*m_Logger)(CString strFormat, ...);

	CString Unescape(CString strValue)
	{
		CString strUnescaped = strValue;
		if (strUnescaped.Find(L"%") >= 0)
		{
			for (map<CString, CString>::iterator iter = m_Globals.begin(); iter != m_Globals.end(); iter++)
				strUnescaped.Replace(L"%" + iter->first + L"%", iter->second);
		}
		if ((strUnescaped.Mid(1, 2) == L":\\") || (strUnescaped.Mid(1, 2) == L":/"))
			strUnescaped.Replace(L"/", L"\\");
		return strUnescaped;
	}

	CString UnescapePath(CString strPath)
	{
		CString strUnescaped = strPath;
		strUnescaped.Replace(L"/", L"\\");
		return Unescape(strUnescaped);
	}

	vector<map<CString, CString>> GetActions(CString strTypeName)
	{
		vector<map<CString, CString>> result;
		for (size_t i = 0; i < m_OnInstall.size(); i++)
		{
			map<CString, CString> shortcut;
			if (m_OnInstall[i].find(L"type") == m_OnInstall[i].end())
				continue;
			if (m_OnInstall[i][L"type"] != strTypeName)
				continue;

			map<CString, CString>::iterator iter;
			for (iter = m_OnInstall[i].begin(); iter != m_OnInstall[i].end(); iter++)
			{
				shortcut[iter->first] = iter->second;
			}

			result.push_back(shortcut);
		}
		return result;
	}

	void Log(CString strFormat, ...)
	{
		if (m_Logger != NULL)
		{
			CString s;

			va_list argList;
			va_start(argList, strFormat);
			s.FormatV(strFormat, argList);
			va_end(argList);

			m_Logger(s);
		}
	}

public:
	void SetLogPrefix(CString key, CString prefix)
	{
		m_LogFormats[key] = prefix;
	}

	void SetLogger(void (*logger)(CString, ...))
	{
		m_Logger = logger;
	}

	BOOL ParseScript(CString strJsonFile)
	{
		m_Globals = getWellknownTags(); //key is lowercase

		map<CString, CString> variables = GetRootInfo(strJsonFile); //key is lowercase

		int nLastUnresoved = 0;
		while(variables.size() > 0)
		{
			int nUnresovedTotal = 0;

			vector<CString> toDelete;

			for (map<CString, CString>::iterator iter = variables.begin(); iter != variables.end(); iter++)
			{
				CStringArray aryTags;
				SplitString(aryTags, iter->second, L"%", TRUE);

				int nUnresoved = 0;
				for (int j = 0; j < aryTags.GetCount(); j++)
				{
					if ((j & 1) > 0)
					{
						CString lowerTag = aryTags[j];
						lowerTag.MakeLower();

						iter->second.Replace(L"%" + aryTags[j] + L"%", L"%" + lowerTag + L"%"); //make tag name lowercase

						if (m_Globals.find(lowerTag) != m_Globals.end())
						{ //known tag
							iter->second.Replace(L"%" + lowerTag + L"%", m_Globals[lowerTag]); //make tag name lowercase
						}
						else
						{ //unknown tag
							nUnresoved++;
							nUnresovedTotal++;
						}
					}
				}

				if (nUnresoved == 0)
				{
					CString strKey = iter->first;
					strKey.MakeLower();
					toDelete.push_back(strKey);

					m_Globals[strKey] = iter->second;
				}
			}

			for (size_t j = 0; j < toDelete.size(); j++)
			{
				variables.erase(variables.find(toDelete[j]));
			}

			if (nLastUnresoved == nUnresovedTotal)
				break;
			nLastUnresoved = nUnresovedTotal;
		}

		if (nLastUnresoved != 0)
			return FALSE;

		if (!GetOnInit(strJsonFile, m_OnInit))
			return FALSE;

		if (!GetOnInstall(strJsonFile, m_OnInstall))
			return FALSE;

		if (!GetOnUninstall(strJsonFile, m_OnUninstall))
			return FALSE;

		return TRUE;
	}

	void SetInstallFolder(CString strFolder)
	{
		m_Globals[L"install_folder"] = FormatPath(strFolder, 0, -1);
	}

	CString GetInstallFolder()
	{
		if (m_Globals.find(L"install_folder") == m_Globals.end())
			return L"";

		CString strFolder = m_Globals[L"install_folder"];
		strFolder = UnescapePath(strFolder);
		return FormatPath(strFolder, 0, -1); //end without slash
	}

	CString GetInstallName()
	{
		if (m_Globals.find(L"install_name") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"install_name"]);
	}

	CString GetPlatform()
	{
		if (m_Globals.find(L"platform") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"platform"]);
	}

	CString GetLanguage()
	{
		if (m_Globals.find(L"language") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"language"]);
	}

	CString GetPublisher()
	{
		if (m_Globals.find(L"publisher") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"publisher"]);
	}

	CString GetProductName()
	{
		if (m_Globals.find(L"product_name") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"product_name"]);
	}

	CString GetProductVersion()
	{
		if (m_Globals.find(L"product_version") == m_Globals.end())
			return L"";

		return Unescape(m_Globals[L"product_version"]);
	}

	void AppendRecyle(CString path)
	{
		m_aryRecyleList.push_back(path);
	}

	void Cleanup()
	{
		for (size_t i = 0; i < m_aryRecyleList.size(); i++)
		{
			try
			{
				if (_waccess(m_aryRecyleList[i], 0) >= 0)
				{
					DeleteFile(m_aryRecyleList[i]);
				}
			}
			catch (CException* e)
			{
				e->Delete();
			}
		}
	}

	void CompressTest()
	{
		CString strFolder = L"C:\\test≤‚ ‘";
		CStringArray aryFileList;
		CInstallUtil::ScanFolder(strFolder, aryFileList);
		for (int i = 0; i < aryFileList.GetCount(); i++)
		{
			TRACE(L"%s\r\n", aryFileList[i]);
		}

		CString strPackFile = CInstallUtil::GetTempFile(L"pack-", L".pack");
		CString strZipFile = CInstallUtil::GetTempFile(L"pack-", L".zip");
		CInstallUtil::PackingFiles(aryFileList, strFolder.GetLength(), strPackFile);

		BOOL bPackResult = CInstallUtil::CompressFile(strPackFile, strZipFile);

		CString strOutPackFile = CInstallUtil::GetTempFile(L"pack-", L".pack");
		BOOL bUnpackResult = CInstallUtil::UncompressFile(strZipFile, strOutPackFile);

		CInstallUtil::AppendRecyle(strPackFile);
		CInstallUtil::AppendRecyle(strZipFile);
		CInstallUtil::AppendRecyle(strOutPackFile);
	}

	BOOL UnpackToFolder(CString strPackFile)
	{
		CFile packFile;
		FileIndex fIndex;
		wchar_t szPath[MAX_PATH];

		packFile.Open(strPackFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, NULL);
		if (packFile.m_hFile == CFile::hFileNull)
			return FALSE;

		LONGLONG lReadPos = 0;
		int nBlockSize = 8 * 1024;
		BYTE* pBuffer = new BYTE[nBlockSize];
		if (pBuffer != NULL)
		{
			CString strSetupList;
			strSetupList.Format(L"%s\\.setup\\.setup", GetInstallFolder());
			CreateDirectoryForPath(strSetupList);

			CStdioFile fSetup;
			fSetup.Open(strSetupList, CFile::modeCreate | CFile::modeReadWrite | CFile::typeUnicode | CFile::shareDenyNone, NULL);

			while (TRUE)
			{
				packFile.Seek(lReadPos, CFile::begin);
				int nSize = packFile.Read(&fIndex, sizeof(fIndex));
				if (nSize < sizeof(fIndex))
					break;

				memset(szPath, 0, sizeof(szPath));
				packFile.Read(szPath, fIndex.offsetContent - sizeof(fIndex));

				CString destFile = GetInstallFolder() + szPath;
				Log(L"    " + destFile);
				CreateDirectoryForPath(destFile);

				if (fSetup.m_hFile != CFile::hFileNull)
				{
					CString strFilePath;
					strFilePath.Format(L"%s,%s\r\n", destFile, fIndex.szMD5);
					fSetup.WriteString(strFilePath);
				}

				CFile fWrite;
				fWrite.Open(destFile, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyNone, NULL);

				while (fIndex.nContentSize > 0)
				{
					nSize = packFile.Read(pBuffer, (UINT)min(nBlockSize, fIndex.nContentSize));
					if (nSize <= 0)
						break;

					if (fWrite.m_hFile != CFile::hFileNull)
						fWrite.Write(pBuffer, nSize);

					fIndex.nContentSize -= nSize;
				}

				if (fWrite.m_hFile != CFile::hFileNull)
					fWrite.Close();

				CFileStatus status;
				CFile::GetStatus(destFile, status);
				status.m_mtime = fIndex.tLastModified;
				CFile::SetStatus(destFile, status);

				lReadPos = fIndex.offsetNextFile;
			}

			if (fSetup.m_hFile != CFile::hFileNull)
				fSetup.Close();

			delete[] pBuffer;
		}

		packFile.Close();
		return TRUE;
	}

	CString GetRunning()
	{
		CString strRunning = L"";
		vector<map<CString, CString>> result;
		for (size_t i = 0; i < m_OnInit.size(); i++)
		{
			if (m_OnInit[i].find(L"type") == m_OnInit[i].end())
				continue;
			if (wcsicmp(m_OnInit[i][L"type"], L"check_running") != 0)
				continue;

			if (m_OnInit[i].find(L"target") != m_OnInit[i].end())
			{
				CString strUnescape = UnescapePath(m_OnInit[i][L"target"]);
				if (_wcsicmp(strUnescape, GetCodePath()) != 0)
				{
					if (IsRunning(strUnescape))
					{
						if (!strRunning.IsEmpty())
							strRunning += L"\r\n";
						strRunning += strUnescape;
					}
				}
			}
		}
		return strRunning;
	}

	CString GetUninstaller()
	{
		HKEY  hKey;
		CString strKey;
		strKey.Format(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s", GetInstallName());

		REGSAM Rights = KEY_ALL_ACCESS;
		int osBits = GetNativeSystemProcessorArchitecture();
		if (osBits == 64)
			Rights |= KEY_WOW64_64KEY;

		if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, strKey, 0, Rights, &hKey))
		{
			DWORD cbData = 512;
			wchar_t szRegInfo[2048];
			memset(szRegInfo, 0, sizeof(szRegInfo));

			if (ERROR_SUCCESS == RegQueryValueExW(hKey, L"UninstallString", NULL, NULL, (BYTE*)szRegInfo, &cbData))
			{
				RegCloseKey(hKey);
				return FormatPath(szRegInfo, 0, 0);
			}

			RegCloseKey(hKey);
		}
		return L"";
	}

	void Install()
	{
		RunScript(m_OnInstall);
	}

	void Uninstall()
	{
		CString strUninstaller = GetUninstaller();
		strUninstaller.Replace(L"/", L"\\");

		RunScript(m_OnUninstall);

		CString strCodebase = CInstallUtil::GetCodeBase();
		int idx = strUninstaller.ReverseFind('\\');
		if (idx >= 0)
			strCodebase = strUninstaller.Left(idx+1);

		map<CString, map<CString, CString>> installedFiles = ScanInstalledFiles(strCodebase); //path ==> setupMD5/MD5

		map<CString, CString> md5;
		installedFiles[strCodebase + L".setup\\.setup"] = md5;
		for (map<CString, map<CString, CString>>::iterator iter = installedFiles.begin(); iter != installedFiles.end(); iter++)
		{
			Log(L"   %s", iter->first);
			try
			{
				if (wcsicmp(strUninstaller, iter->first) == 0)
				{
				}
				else
				{
					if (iter->second[L"setupMD5"] == iter->second[L"MD5"])
						DeleteFileW(iter->first);
				}
			}
			catch (CException* e)
			{
				e->Delete();
			}
			Sleep(10);
		}

		DeleteDirectory(strCodebase, TRUE);

		strCodebase = FormatPath(strCodebase, 0, -1);

		CString strCmd = L"cmd.exe /c choice /C Y /N /D Y /T 3 & Del \"" + strUninstaller + L"\" & rmdir \"" + strCodebase + L"\"";
		ExecuteCommandLine(strCmd, 0, FALSE, FALSE);

		ExitProcess(0);
	}

	void RegAction(map<CString, CString>& action, const char* type)
	{
		CString strType(type);

		if (action.find(L"parent") == action.end())
			return;
		if (action.find(L"key") == action.end())
			return;

		REGSAM Rights = KEY_ALL_ACCESS;
		if (action.find(L"platform") != action.end())
		{
			int osBits = GetNativeSystemProcessorArchitecture();
			if (osBits == 64)
			{
				if (wcsicmp(action[L"platform"], L"os") == 0)
					Rights |= KEY_WOW64_64KEY;
				else if (wcsicmp(action[L"platform"], L"x86") == 0)
					Rights |= KEY_WOW64_32KEY;
				else if (wcsicmp(action[L"platform"], L"x64") == 0)
					Rights |= KEY_WOW64_64KEY;
			}
		}

		HKEY hRootKey = HKEY_LOCAL_MACHINE;
		CString strParent = Unescape(action[L"parent"]);
		if (wcsicmp(strParent, L"HKEY_CLASSES_ROOT") == 0)
			hRootKey = HKEY_CLASSES_ROOT;
		else if (wcsicmp(strParent, L"HKEY_CURRENT_USER") == 0)
			hRootKey = HKEY_CURRENT_USER;
		else if (wcsicmp(strParent, L"HKEY_USERS") == 0)
			hRootKey = HKEY_USERS;
		else if (wcsicmp(strParent, L"HKEY_LOCAL_MACHINE") == 0)
			hRootKey = HKEY_LOCAL_MACHINE;

		CString strKey = UnescapePath(action[L"key"]);

		HKEY  hKey;
		wchar_t szValue[2048];

		if (strType == "regdel")
		{
			LSTATUS result = RegDeleteKeyEx(hRootKey, strKey, Rights, 0);
			if (ERROR_SUCCESS != result)
			{
			}
		}
		else if (strType == "regadd")
		{
			CString strFormat = m_LogFormats[L"registry"];
			Log(strFormat, strKey);

			if (ERROR_SUCCESS == RegCreateKeyExW(hRootKey, strKey, 0, NULL, REG_OPTION_NON_VOLATILE, Rights, NULL, &hKey, NULL))
			{
				map<CString, CString>::iterator iter;
				for (iter = action.begin(); iter != action.end(); iter++)
				{
					CString strName = iter->first;
					if (wcsicmp(strName, L"type") == 0)
						continue;
					if (wcsicmp(strName, L"parent") == 0)
						continue;
					if (wcsicmp(strName, L"key") == 0)
						continue;
					if (wcsicmp(strName, L"platform") == 0)
						continue;

					CString strValue = Unescape(iter->second);
					if ((strValue.Mid(1, 2) == L":/") || (strValue.Mid(1, 2) == L":\\"))
						strValue.Replace(L"/", L"\\");

					wcscpy(szValue, strValue);
					if (ERROR_SUCCESS != RegSetValueExW(hKey, strName, 0, REG_SZ, (BYTE*)szValue, (wcslen(szValue) + 1) * sizeof(wchar_t)))
					{
					}
				}
				RegCloseKey(hKey);
			}
		}
	}

	void RunScript(vector<map<CString, CString>> actions)
	{
		for (size_t i = 0; i < actions.size(); i++)
		{
			if (actions[i].find(L"type") == actions[i].end())
				continue;

			CString strType = actions[i][L"type"];
			if (wcsicmp(strType, L"check_running") == 0)
				continue;

			if (wcsicmp(strType, L"shortcut") == 0)
			{
				CString strName = Unescape(actions[i][L"name"]);
				CString strLink = UnescapePath(actions[i][L"link"]);
				CString strTarget = UnescapePath(actions[i][L"target"]);

				CString strFolder = L"";
				if (actions[i].find(L"startin") != actions[i].end())
					strFolder = UnescapePath(actions[i][L"startin"]);

				CString strIcon = L"";
				if (actions[i].find(L"icon") != actions[i].end())
					strIcon = UnescapePath(actions[i][L"icon"]);

				CString strFormat = m_LogFormats[L"shortcut"];
				Log(strFormat, strLink);

				CreateLink(strName, strLink, strTarget, strFolder, strIcon);
			}
			else if (wcsicmp(strType, L"regdel") == 0)
			{
				RegAction(actions[i], "regdel");
			}
			else if (wcsicmp(strType, L"regadd") == 0)
			{
				RegAction(actions[i], "regadd");
			}
			else if (wcsicmp(strType, L"execute") == 0)
			{
				BOOL bVisible = FALSE;
				CString strCmd = L"";
				CString strArguments = L"";

				if (actions[i].find(L"visible") != actions[i].end())
				{
					CString strVisible = L"";
					strVisible = actions[i][L"visible"];
					strVisible.Trim();
					strVisible.MakeLower();
					if ((strVisible == L"true") || (strVisible == L"yes") || (_wtol(strVisible) > 0))
					{
						bVisible = TRUE;
					}
				}
				if (actions[i].find(L"command") != actions[i].end())
				{
					strCmd = actions[i][L"command"];
					strCmd.Trim();
					strCmd.Replace(L"'", L"\"");

					strCmd = Unescape(strCmd);
				}
				if (actions[i].find(L"parameters") != actions[i].end())
				{
					strArguments = actions[i][L"parameters"];
					strArguments.Trim();
					strArguments.Replace(L"'", L"\"");

					strArguments = Unescape(strArguments);
				}

				if (strCmd.IsEmpty())
					continue;

				CProcessExcutor* pExecutor = new CProcessExcutor();
				if (pExecutor)
				{
					pExecutor->SetVisible(bVisible);
					pExecutor->SetRedirect(TRUE);
					pExecutor->SetWait(TRUE);

					if (strCmd.Left(1) != L"\"")
						strCmd = L"\"" + strCmd;
					if (strCmd.Right(1) != L"\"")
						strCmd += L"\"";

					CString strFormat = m_LogFormats[L"execute"];
					Log(strFormat, strCmd + L" " + strArguments);

					pExecutor->SetCommand(strCmd, strArguments);

					pExecutor->Start(this);

					WaitForSingleObject(pExecutor->GetThreadHandle(), INFINITE);

					delete pExecutor;
				}
			}
		}
	}
};

#endif
