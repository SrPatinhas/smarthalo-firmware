
@Echo off
copy C:\SmartHalo_git\Code\Firmware\shapp\_build\shapp.hex application.hex 
copy C:\SmartHalo_git\Code\3rd\nrf5_sdk\nRF5_SDK_12.1.0_0d23e2a\components\softdevice\s132\hex\s132_nrf52_3.0.0_softdevice.hex  softdevice.hex
nrfutil settings generate --family NRF52 --application application.hex --application-version 0 --bootloader-version 2 --bl-settings-version 1 settings.hex
mergehex --merge application.hex softdevice.hex --output merged.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program merged.hex
nrfjprog --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5