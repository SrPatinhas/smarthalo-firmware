
@Echo off
copy ..\..\SmartHalo_SDK11_Firmware\_build\s132_pca10040.hex application.hex 
copy ..\..\SmartHalo_BLE_Bootloader\_build\nrf52832_xxaa_s132.hex BLE_BOOT.hex
mergehex --merge application.hex s132_nrf52_2.0.0_softdevice.hex --output merged.hex
mergehex --merge merged.hex BLE_BOOT.hex --output Boot_s132_firm_merge.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program Boot_s132_firm_merge.hex
nrfjprog --family nRF52 --memwr 0x7F000 --val 1
nrfjprog --family nRF52 --pinreset
nrfjprog --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5