
@Echo off

nrfjprog -s 682841002 --family nRF52 -e
nrfjprog -s 682841002 --family nRF52 --program connectivity_115k2_with_s132_2.0.1.hex
nrfjprog -s 682841002 --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5