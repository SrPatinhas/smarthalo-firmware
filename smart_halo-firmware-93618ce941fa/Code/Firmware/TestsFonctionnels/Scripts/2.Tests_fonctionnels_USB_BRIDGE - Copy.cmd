
@Echo off
setlocal enableextensions enabledelayedexpansion

set serialKey=USB_BLE_BRIDGE_board_serial_number
set applicationKey=USB_BLE_BRIDGE
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
            if "x!serialKey!"=="x!currkey!" (          
		set SerialVal=%%c
            )
        )
    )
)

nrfjprog -s !SerialVal! --family nRF52 -e
nrfjprog -s !SerialVal! --family nRF52 --program !applicationVal!
nrfjprog -s !SerialVal! --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5