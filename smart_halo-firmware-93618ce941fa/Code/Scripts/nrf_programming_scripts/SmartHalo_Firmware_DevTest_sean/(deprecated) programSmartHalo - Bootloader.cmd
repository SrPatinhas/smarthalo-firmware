
@Echo off
copy ..\..\SmartHalo_SDK11_DUAL_Bootloader\_build\nrf52832_xxaa_s132.hex DUAL_BOOT.hex
mergehex --merge DUAL_BOOT.hex s132_nrf52_2.0.0_softdevice.hex --output Boot_s132_no_app.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program Boot_s132_no_app.hex
nrfjprog --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5