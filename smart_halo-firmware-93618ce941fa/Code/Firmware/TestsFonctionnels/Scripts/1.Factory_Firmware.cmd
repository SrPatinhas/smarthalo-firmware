
@Echo off

nrfutil settings generate --family NRF52 --application application.hex --application-version 0 --bootloader-version 2 --bl-settings-version 1 settings.hex
mergehex --merge application.hex softdevice.hex --output merged.hex
mergehex --merge merged.hex settings.hex --output merged_settings.hex
mergehex --merge merged_settings.hex BOOT.hex --output Boot_s132_firm_merge.hex
nrfjprog -s 164205407 --family nRF52 -e
nrfjprog -s 164205407 --family nRF52 --program Boot_s132_firm_merge.hex
nrfjprog -s 164205407 --family nRF52 --pinreset
nrfjprog -s 164205407 --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5