import gatt
import time
from subprocess import call
import os
import sys
import datetime

manager = gatt.DeviceManager(adapter_name='hci0')

class AnyDevice(gatt.Device):
    def connect_succeeded(self):
        super().connect_succeeded()
        print("[%s] Connected" % (self.mac_address))

    def connect_failed(self, error):
        super().connect_failed(error)
        print("[%s] Connection failed: %s" % (self.mac_address, str(error)))

    def disconnect_succeeded(self):
        super().disconnect_succeeded()
        print("[%s] Disconnected" % (self.mac_address))
        time.sleep(1)
        manager.stop()

    def services_resolved(self):
        super().services_resolved()

        print("[%s] Resolved services" % (self.mac_address))

        for service in self.services:
            # print("[%s]  Service [%s]" % (self.mac_address, service.uuid))
            for characteristic in service.characteristics:
                # print("[%s]    Characteristic [%s]" % (self.mac_address, characteristic.uuid))
                values = []
                characteristic_value = characteristic.read_value()
                if characteristic_value != None:
                    for value in characteristic.read_value():
                        values.append(int(value))
                    if '92540001' in characteristic.uuid:
                        control = characteristic
                        characteristic.enable_notifications()
                    elif '92540010' in characteristic.uuid:
                        payload0 = characteristic
                    elif '92540011' in characteristic.uuid:
                        payload1 = characteristic
                    elif '92540012' in characteristic.uuid:
                        payload2 = characteristic
                    elif '92540013' in characteristic.uuid:
                        payload3 = characteristic
                    elif '92540101' in characteristic.uuid:
                        upControl = characteristic
                    elif '92540111' in characteristic.uuid:
                        upPayload = characteristic
                # else:
                #     print("No Values")        

        self.getVersionData(control, payload0, payload1, payload2, payload3)
        self.getSerialData(control, payload0, payload1, payload2, payload3)
        self.getPCBAData(control, payload0, payload1, payload2, payload3)
        self.getKeyData(control, payload0, payload1, payload2, payload3)
        self.getPublicKey(control, payload0, payload1, payload2, payload3)

        #Authenticate Device
        self.authentication(control, payload0, payload1, payload2, payload3)

        self.getBatteryData(control, payload0, payload1, payload2, payload3)        

        print(datetime.datetime.now())

        # Start Bootloader
        self.sendToBootloader(control, payload0, payload1, payload2, payload3)

        # send new firmware
        os.system("sudo nrfutil dfu ble -pkg shapp_1.7.2.0_180719-163114.zip -ic NRF52 -n 'SH_BL' -p /dev/ttyACM0")

        device.disconnect()


    # def characteristic_value_updated(self, characteristic, value):
    #     if '92540001' in characteristic.uuid:
    #         value_info = []
    #         for v in value:
    #             value_info.append(v)
    #         print(value_info)
 
    # def characteristic_write_value_succeeded(self, characteristic):
    #     if '92540001' in characteristic.uuid:
    #         print("Write Succeeded! Characteristic: ", characteristic.uuid)

    def characteristic_write_value_failed(self, characteristic, error):
        print("Write Failed!, Error: ", str(error))

    def transcieve(self, cmd, encrypted, control, payload0, payload1, payload2, payload3, response_length, isNumeric=True, isResponse=True):
        length = len(cmd)
        if(length > 60):
            payload3.write_value(cmd[60:])
        if(length > 40):
            payload2.write_value(cmd[40:61])
        if(length > 20):
            payload1.write_value(cmd[20:41])
        payload0.write_value(cmd[0:21])

        control.write_value([length, encrypted])

        time.sleep(1)
        # return_info = []
        # for value in control.read_value():
        #     return_info.append(int(value))

        if(isResponse):
            return_values = self.checkStatus(20,payload0,isNumeric)
            if len(return_values) > 0:
                return_values += self.readCharacteristic(20,payload1,isNumeric)
                return_values += self.readCharacteristic(20,payload2,isNumeric)
                return_values += self.readCharacteristic(20,payload3,isNumeric)
            return return_values[1:response_length]

    def checkStatus(self, length, characteristic,isNumeric=True):
        response = self.readCharacteristic(length, characteristic)
        if response[0] == 0:
            # Response is OK
            if isNumeric:
                return response
            else:
                str_response = ""
                for i in range(len(response)):
                    str_response += chr(response[i])
                return str_response
        else:
            # Response is fail
            if isNumeric:
                return []
            else: 
                return ""

    def readCharacteristic(self, length, characteristic, isNumeric=True):
        if isNumeric:
            values = []
        else:
            values = ""
        for value in characteristic.read_value():
            if isNumeric:
                values.append(int(value))
            else:
                values += chr(value)
            if len(values) == length:
                return values

    def getVersionData(self, control, payload0, payload1, payload2, payload3):
        versions = self.transcieve([0,0],False,control, payload0, payload1, payload2, payload3,11,True)
        print("FW: " + str(versions[0]) + "." + str(versions[1]) + "." + str(versions[2]) + "." + str(versions[3]))
        print("BL: " + str(versions[4]) + "." + str(versions[5]) + "." + str(versions[6]) + "." + str(versions[7]))
        print("HW: " + self.getHardwareVersion(versions[8]))
        print("Acc: " + self.getAccVersion(versions[9]))
        return versions

    def getHardwareVersion(self, value):
        if value == 0:
            return "1.0/1.1"
        return "1.2"

    def getAccVersion(self, value):
        if value == 0:
            return "CTR"
        return "AGR"

    def getSerialData(self, control, payload0, payload1, payload2, payload3):
        serial = self.transcieve([0,5,0],False,control,payload0, payload1, payload2, payload3,10,False)       
        print("Serial: " + serial)
        return serial

    def getPCBAData(self, control, payload0, payload1, payload2, payload3):
        pcba = self.transcieve([0,5,1],False,control,payload0, payload1, payload2, payload3,13,False)
        print("PCBA: " + pcba)
        return pcba

    def getKeyData(self, control, payload0, payload1, payload2, payload3):
        key = self.transcieve([0,5,2],False,control, payload0, payload1, payload2, payload3,13,False)
        print("Key: " + key)
        return key

    def getPublicKey(self, control, payload0, payload1, payload2, payload3):
        public = self.transcieve([0,1],False,control, payload0, payload1, payload2, payload3,69,True)
        public_hex = ""
        for value in public:
            public_hex += '{:02x}'.format(value,'x')
        print("Public: " + public_hex[:16].upper())    
        print("Public Brief: " + public_hex[12:16].upper())    
        return public_hex[:16]

    def authentication(self, control, payload0, payload1, payload2, payload3):
        #Authenticate Device
        self.transcieve([0,3],False,control, payload0, payload1, payload2, payload3,0,True)

    def getBatteryData(self, control, payload0, payload1, payload2, payload3):
        # Check Status
        status = self.transcieve([4,1],False,control, payload0, payload1, payload2, payload3,10,True)
        print("Battery: " + str(status[0]))
        return status[0]

    def sendToBootloader(self, control, payload0, payload1, payload2, payload3):
        # Start Bootloader
        self.transcieve([4,0],False,control, payload0, payload1, payload2, payload3,0,True,False)

# e1:a5:47:29:11:4f FB9C
# ce:5b:8c:a1:f0:de 178D
# fd:30:32:bc:3e:c4 40da
# f8:0b:0a:56:de:ce 6c00
# ec:35:99:59:50:10 b215
if len(sys.argv) > 1:
    mac = sys.argv[1]
else:
    mac = 'fa:6c:3d:ad:7a:b8'

device = AnyDevice(mac_address=mac, manager=manager)
device.connect()

manager.run()