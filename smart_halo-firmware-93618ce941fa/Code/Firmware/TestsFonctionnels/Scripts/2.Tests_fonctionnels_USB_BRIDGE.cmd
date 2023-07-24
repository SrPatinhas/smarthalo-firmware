
@Echo off

nrfjprog -s 682736524 --family nRF52 -e
nrfjprog -s 682736524 --family nRF52 --program USB_BLE_BRIDGE_V1.0.hex
nrfjprog -s 682736524 --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5