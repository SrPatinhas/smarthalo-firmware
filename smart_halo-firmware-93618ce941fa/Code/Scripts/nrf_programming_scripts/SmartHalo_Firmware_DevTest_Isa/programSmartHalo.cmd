

@Echo off
copy ..\..\SmartHalo_SDK11_Firmware\_build\s132_pca10040.hex application.hex 
mergehex --merge application.hex s132_nrf52_2.0.0_softdevice.hex --output merged.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program merged.hex
nrfjprog --family nRF52 -r
IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully
