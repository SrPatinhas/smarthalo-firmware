
@Echo off
copy C:\SmartHalo_git\Code\Firmware\TestsFonctionnels\_build\TestsFonctionnels.hex application.hex 
copy C:\SmartHalo_git\Code\3rd\nrf5_sdk\nRF5_SDK_12.1.0_0d23e2a\components\softdevice\s132\hex\s132_nrf52_3.0.0_softdevice.hex softdevice.hex
copy C:\SmartHalo_git\Code\Firmware\shbootloader\_build\nrf52832_xxaa_s132.hex BOOT.hex

nrfutil settings generate --family NRF52 --application application.hex --application-version 0 --bootloader-version 2 --bl-settings-version 1 settings.hex
mergehex --merge application.hex softdevice.hex --output merged.hex
mergehex --merge merged.hex settings.hex --output merged_settings.hex
mergehex --merge merged_settings.hex BOOT.hex --output Boot_s132_firm_merge.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program Boot_s132_firm_merge.hex
nrfjprog --family nRF52 --pinreset
nrfjprog --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5