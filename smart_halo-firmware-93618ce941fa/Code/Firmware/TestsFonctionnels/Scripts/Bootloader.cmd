
@Echo off

mergehex --merge bootloaderV0.1.hex softdevice_S132_V3.hex --output merged.hex
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program merged.hex
nrfjprog --family nRF52 -r

IF %ERRORLEVEL% GTR 0 Echo An error has occured, SmartHalo wasn't programmed successfully
IF %ERRORLEVEL% EQU 0 Echo SmartHalo was programmed successfully

sleep 5