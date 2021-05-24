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

# Batch File to Build Installer

* build-setup-en-x86.bat or build-setup-en-x64.bat
* Installer is a x86 program even though your application is x64

