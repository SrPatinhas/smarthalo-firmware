
@Echo off
setlocal enableextensions enabledelayedexpansion

set applicationKey=app_dfu_package
set comPortKey=ComPort_for_DFU
set currarea=
for /f "usebackq delims=" %%a in ("Hex_file_names.ini") do (
    set ln=%%a
    if "x!ln:~0,1!"=="x[" (
        set currarea=!ln!
    ) else (
        for /f "tokens=1,2 delims==" %%b in ("!ln!") do (
            set currkey=%%b
            set currval=%%c
            if "x!applicationKey!"=="x!currkey!" (          
		set applicationVal=%%c
            )
        )
    )
)
for /f "usebackq delims=" %%a in ("JLINK_serial_numbers_comPorts.ini") do (
    set ln=%%a
    if "x!ln:~0,1!"=="x[" (
        set currarea=!ln!
    ) else (
        for /f "tokens=1,2 delims==" %%b in ("!ln!") do (
            set currkey=%%b
            set currval=%%c
            if "x!comPortKey!"=="x!currkey!" (          
		set comPortVal=%%c
            )
        )
    )
)

nrfutil dfu ble -pkg !applicationVal! -p !comPortVal! -n "DfuTarg" 

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5