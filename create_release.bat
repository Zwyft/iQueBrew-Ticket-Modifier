@echo off
set "InputPath=aulon\dist\ique_modern.exe"
set "ImagesPath=aulon\images"
set "ReleaseDir=dist\release"
set "ReleaseZip=dist\iQueBrew_Ticket_Editor_v1.0.zip"

echo Preparing Release...

if exist "%ReleaseDir%" rd /s /q "%ReleaseDir%"
mkdir "%ReleaseDir%"
mkdir "%ReleaseDir%\images"
mkdir "%ReleaseDir%\roms"

echo Copying Executable...
copy "%InputPath%" "%ReleaseDir%\iQueBrew_Ticket_Editor.exe"

echo Copying Assets...
xcopy /E /I /Y "%ImagesPath%" "%ReleaseDir%\images"
copy README.md "%ReleaseDir%\README.txt"

echo Creating Zip Package...
powershell -command "Compress-Archive -Path '%ReleaseDir%\*' -DestinationPath '%ReleaseZip%' -Force"

echo Done! Release Package at: %ReleaseZip%
pause
