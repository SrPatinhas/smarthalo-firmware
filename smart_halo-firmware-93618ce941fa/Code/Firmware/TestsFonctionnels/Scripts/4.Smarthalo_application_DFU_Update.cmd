
@Echo off

nrfutil dfu ble -pkg shapp_dfu_package_20161110.zip -p COM6 -n "DfuTarg" 

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5