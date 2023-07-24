test: 
	gcc main.c HAL/peripheral.c HAL/AccGyro/Peripheral_AccGyro_test.c HAL/WhiteLED/Peripheral_WhiteLED_test.c HAL/HaloLED/Peripheral_HaloLED_test.c HAL/Piezo/Peripheral_Piezo_test.c SmartHaloOS/SmartHaloOS.c SmartHaloOS/MessageBroker.c SmartHaloOS/Graphics.c SmartHaloOS/Sounds.c Applications/Alarm/Alarm.c

sh2: 
	gcc main.c HAL/peripheral.c HAL/AccGyro/Peripheral_AccGyro_lsm303agr.c SmartHaloOS/SmartHaloOS.c 

clean:
	-rm a.out main.o


