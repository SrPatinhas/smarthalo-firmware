
@Echo off
setlocal enableextensions enabledelayedexpansion

set CypressSettingsKey=Cypress_settings
set currarea=
for /f "usebackq delims=" %%a in ("Hex_file_names.ini") do (
    set ln=%%a
    if "x!ln:~0,1!"=="x[" (
        set currarea=!ln!
    ) else (
        for /f "tokens=1,2 delims==" %%b in ("!ln!") do (
            set currkey=%%b
            set currval=%%c
            if "x!CypressSettingsKey!"=="x!currkey!" (          
		set SettingsVal=%%c
            )
        )
    )
)

fwDownload.exe -vid 0x04B4 -pid 0x0003 -mode vendor -c !SettingsVal!

sleep 5