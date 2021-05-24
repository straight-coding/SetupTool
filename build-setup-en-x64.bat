REM set path of MSBuild.exe
REM SET MSBUILD="%programfiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
SET MSBUILD="D:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"

REM set path of install source
SET COMPILE_SOURCE="C:\Program Files\Windows NT\Accessories"
SET INSTALL_SOURCE=.\setup-image\enx64
%systemroot%\System32\xcopy %COMPILE_SOURCE% %INSTALL_SOURCE%\ /s /e /h /y

copy /y /b .\setup-en-x64.json .\Installer\setup.json
copy /y /b .\setup-en-x64.json .\Uninstaller\setup.json

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

copy /y /b .\Installer\Release\Installer.exe .\setup-en-x64.exe

pause
