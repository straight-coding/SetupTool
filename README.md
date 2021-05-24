# A DIY Installer Builder

* Build your own installer using Microsoft Visual Studio 2019
* The configuration of the installer is a JSON file

# Script File

* setup-en-x86.json or setup-en-x64.json
```
{
  "product_name": "Setup Demo",
  "product_version": "1.0.0.1",
  "publisher": "Straight",
  "language": "en",
  "platform": "x86",
  "install_name": "%product_name%(%Language%%platform%)",
  "install_folder": "%SystemDrive%/%install_name%",
  "on_init": [
    {
      "type": "check_running",
      "target": "%install_folder%/wordpad.exe"
    },
    {
      "type": "check_running",
      "target": "%install_folder%/Uninstaller.exe"
    }
  ],
  "on_install": [
    {
      "type": "shortcut",
      "name": "%install_name%",
      "link": "%SystemDrive%/Users/Public/Desktop/%install_name%.lnk",
      "target": "%install_folder%/wordpad.exe",
      "icon": "%install_folder%/icon/Installer.ico",
      "startin": "%install_folder%"
    },
    {
      "type": "shortcut",
      "name": "%install_name%",
      "link": "%ProgramData%/Microsoft/Windows/Start Menu/Programs/%install_name%/%install_name%.lnk",
      "target": "%install_folder%/wordpad.exe",
      "icon": "%install_folder%/icon/Installer.ico",
      "startin": "%install_folder%"
    },
    {
      "type": "shortcut",
      "name": "Uninstall %install_name%",
      "link": "%ProgramData%/Microsoft/Windows/Start Menu/Programs/%install_name%/Uninstaller.lnk",
      "target": "%install_folder%/Uninstaller.exe",
      "icon": "%install_folder%/icon/Uninstaller.ico",
      "startin": "%install_folder%"
    },
    {
      "type": "registry",
      "parent": "HKEY_LOCAL_MACHINE",
      "key": "SOFTWARE/Microsoft/Windows/CurrentVersion/Uninstall/%install_name%",
      "DisplayName": "%install_name%",
      "DisplayVersion": "%product_version%",
      "InstallLocation": "%install_folder%",
      "UninstallString": "%install_folder%/Uninstaller.exe",
      "DisplayIcon": "%install_folder%/Uninstaller.exe,0",
      "Publisher": "%publisher%"
    },
    {
      "type": "execute",
      "command": "%install_folder%/prerequisite/VC_redist_2019_x86.exe",
      "parameters": "",
	  "visible": "true"
    }
  ],
  "on_uninstall": [
    {
      "type": "execute",
      "command": "del",
      "parameters": "/Q '%SystemDrive%/Users/Public/Desktop/%install_name%.lnk'"
    },
    {
      "type": "execute",
      "command": "del",
      "parameters": "/Q '%ProgramData%/Microsoft/Windows/Start Menu/Programs/%install_name%/%install_name%.lnk'"
    },
    {
      "type": "execute",
      "command": "del",
      "parameters": "/Q '%ProgramData%/Microsoft/Windows/Start Menu/Programs/%install_name%/Uninstaller.lnk'"
    },
    {
      "type": "execute",
      "command": "rmdir",
      "parameters": "'%ProgramData%/Microsoft/Windows/Start Menu/Programs/%install_name%'"
    },
    {
      "type": "execute",
      "command": "reg.exe",
      "parameters": "delete 'HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows/CurrentVersion/Uninstall/%install_name%' /f"
    }
  ]
}

```

# Batch File to Build the Installer

* build-setup-en-x86.bat or build-setup-en-x64.bat
* Installer is a x86 program even though your application is x64
* The idea is:
  * Copy everything what you want to install to the folder specified by `INSTALL_SOURCE`;
  * Copy the script file (.json) to the source folder of both Installer and Uninstaller;
  * Build Uninstaller and copy it to the folder specified by `INSTALL_SOURCE`;
  * Built the Installer as a compressing tool;
  * Using the above tool to compress the whole folder specified by `INSTALL_SOURCE`;
  * Copy the zip file to the Installer source folder as an embedded file;
  * Re-built the Installer to get the final setup program.

```
REM set path of MSBuild.exe
REM SET MSBUILD="%programfiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
SET MSBUILD="D:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"

REM set path of install source
SET COMPILE_SOURCE="C:\Program Files\Windows NT\Accessories"
SET INSTALL_SOURCE=.\setup-image\enx86
%systemroot%\System32\xcopy %COMPILE_SOURCE% %INSTALL_SOURCE%\ /s /e /h /y

copy /y /b .\setup-en-x86.json .\Installer\setup.json
copy /y /b .\setup-en-x86.json .\Uninstaller\setup.json

REM build library zlib.lib
%MSBUILD% .\zlib\zlib.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x86
copy /y /b .\zlib\Release\zlib.lib .\Release\zlib.lib

REM rebuild Uninstaller.exe
%MSBUILD% .\UnInstaller\UnInstaller.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x86
REM copy Uninstaller.exe as an install source file
copy /y /b .\UnInstaller\Release\UnInstaller.exe %INSTALL_SOURCE%\UnInstaller.exe

REM build Installer.exe first to generate a compressor
%MSBUILD% .\Installer\Installer.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x86

REM Installer.exe works like a compressor to pack install source folder to setup.zip 
REM setup.zip will be embedded as resource in the Installer project
.\Installer\Release\Installer.exe %INSTALL_SOURCE% .\Installer\setup.zip

REM rebuild Installer.exe again with the new setup.zip
%MSBUILD% .\Installer\Installer.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x86

copy /y /b .\Installer\Release\Installer.exe .\setup-en-x86.exe

pause

```

