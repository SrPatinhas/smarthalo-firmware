
@Echo off
setlocal enableextensions enabledelayedexpansion

set serialKey=Segger_programmer_serial_number
set applicationKey=factory_application
set bootloaderKey=bootloader
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
            if "x!bootloaderKey!"=="x!currkey!" (          
		set bootloaderVal=%%c
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

mergehex --merge !applicationVal! !bootloaderKey! --output Boot_s132_firm_merge.hex
nrfjprog -s !SerialVal! --family nRF52 -e
nrfjprog -s !SerialVal! --family nRF52 --program Boot_s132_firm_merge.hex
nrfjprog -s !SerialVal! --family nRF52 --pinreset
nrfjprog -s !SerialVal! --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5