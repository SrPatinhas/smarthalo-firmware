'''
Created on Jul 4, 2016

@author: sgelinas
'''

from abc import ABCMeta, abstractmethod
import random
import time
from SerialBoard import SmartHaloBoard, PCA10040Board
from pip._vendor.html5lib.filters import lint

def generatePRBS(PRBStype, PRBSlength):
        """ Generate a pseudo random binary sequence for loopback test
            purposes
        """
        
        sequence = []
        
        if "counter" in PRBStype:            
            
            while PRBSlength >= 256:
                sequence.extend(range(256))
                PRBSlength -= 256
                        
            if PRBSlength > 0:
                sequence.extend(range(int(PRBSlength)))
                
        elif "random" in PRBStype:            
            
            sequence = [random.choice(range(256)) for _ in range(int(PRBSlength))]
            
        
        return sequence 

class BLEDevice:
    """
        Base class that wraps everything common between
        all types of devices
    """
    
    __metaclass__ = ABCMeta    
    SMARTHALO_GETVERSION_CMD = "VERSION"
    SMARTHALO_RESET_CMD = "RESET"
    SMARTHALO_CMD_SEP = "\r"
    SMARTHALO_DATA_EOL = '\n'        

    DevicePresent = False
    BLEBoard = None
    
    def __init__(self):        
        self.BLEBoard = None
        self.DevicePresent = False             
    
    @abstractmethod
    def getBoard(self):
        pass
        
    def writeCommand(self, command, expectedresponselines=1):
        
        # Clear input buffer so response is appropriate
        self.BLEBoard.clearBuffers()
        # Send command and return response
        return self.BLEBoard.writeCommand(command + self.SMARTHALO_CMD_SEP, expectedresponselines)    
    
    def getVersion(self):
        
        version = None
        
        if self.BLEBoard:
            
            if hasattr(self.BLEBoard, 'isConnected'):      
                if self.BLEBoard.isConnected:
                
                    print "Sending version command to device " + str(self.BLEBoard.boardID)
                    version = self.writeCommand(self.SMARTHALO_GETVERSION_CMD)
                    # Wait for a full response   USB-BLE Bridge, v1.0, Jul  1 2016, 11:04:53
                    #   SmartHalo test program, v1.0, Jul  1 2016, 12:18:46 
        
        return version
    
    def reset(self, expectedlines=5):
        
        incMessage = None
        
        if self.BLEBoard:
            
            if hasattr(self.BLEBoard, 'isConnected'):      
                if self.BLEBoard.isConnected:
                
                    print "resetting device " + str(self.BLEBoard.boardID)
                    incMessage = self.writeCommand(self.SMARTHALO_RESET_CMD,expectedlines)
                    # Wait for a full response  #--------------------------------------------------------
                                                #  SmartHalo test program, v1.1, Aug  1 2016, 11:10:16
                                                #  PCA10040                                                                    
                                                #--------------------------------------------------------                      
                                                #Type (H)elp for a list of commands
        
        return incMessage
    
        
class UUTDevice(BLEDevice):
    """
        Class to wrap everything concerning a Unit Under Test
        which is defined as a unit that runs the TestFirmware
    """
    SMARTHALO_TESTFIRMWARE_ID_STRING = "SmartHalo test program"
    SMARTHALO_LOOPBACKTEST_UART_START_CMD = "startloopbacktest uart"
    SMARTHALO_LOOPBACKTEST_UART_STOP_CMD = b'\x1B' + "STOP"   
    SMARTHALO_RSSI_CMD = "GETRSSIDBM"
    SMARTHALO_TXPOWER_CMD = "SETTXPOWERDBM"     
    
    def __init__(self):        
        super(UUTDevice,self).__init__()
        super(UUTDevice,self).getBoard()
        self.getBoard()
    
    def getBoard(self):        
                
        # Check if we have a Smart Halo Board present
        skiplist = []; 
        self.BLEBoard = SmartHaloBoard()
        currport = self.BLEBoard.findAndOpenPort(skiplist)        
        
        # Loop until we find a SmartHaloBoard running the test firmware or run out of ports  
        while currport != None:
            
            print "Resetting device on port " + str(currport.device)
            print self.reset(6)
            
            version = self.getVersion()        
        
            if version != None:
                print "version for SmartHaloBoard (UUT): " + version
                
                if self.SMARTHALO_TESTFIRMWARE_ID_STRING in version:
                    self.DevicePresent = True 
                    # Reserve the port
                    self.BLEBoard.reserve_port(self.BLEBoard.port)
                    break
                
            if not self.DevicePresent:
                skiplist.append(currport)
                
                if self.BLEBoard.isConnected:
                    self.BLEBoard.disconnect()
                
                currport = self.BLEBoard.findAndOpenPort(skiplist)
        
        # No SmartHaloBoards running the testfirmware. Lets look for PCA10040 boards      
        if not self.DevicePresent:
                        
            skiplist = [];
            self.BLEBoard = PCA10040Board()
            currport = self.BLEBoard.findAndOpenPort(skiplist)
                   
            # Loop until we find a PCABoard running the test firmware or run out of ports   
            while currport != None:   
                
                print "Resetting device on port " + str(currport.device)
                print self.reset(11)     
                    
                version = self.getVersion()
                
                if version != None:            
                    print "version for PCA10040Board (UUT): " + version
                    
                    if self.SMARTHALO_TESTFIRMWARE_ID_STRING in version:
                        self.DevicePresent = True
                        # Reserve the port
                        self.BLEBoard.reserve_port(self.BLEBoard.port)
                        break                    
                         
                if not self.DevicePresent:
                    skiplist.append(currport)
                    
                    if self.BLEBoard.isConnected:
                        self.BLEBoard.disconnect()
                    
                    currport = self.BLEBoard.findAndOpenPort(skiplist)
       
        return self.DevicePresent
    
    def getRSSI(self):
        """ Returns RSSI
        """
        if(self.BLEConnected):
        
            print self.writeLocalCommand(self.SMARTHALO_RSSI_CMD)
                
    def setTxPower(self, txpowerdBm):
        """ Set the local TX power in dBm
            Valid values = -40, -30, -20, -16, -12, -8, -4, 0, 4
        """
        validvalues = [-40, -30, -20, -16, -12, -8, -4, 0, 4]
        
        if(self.DevicePresent):
            if (txpowerdBm in validvalues):
                print self.writeLocalCommand(self.SMARTHALO_TXPOWER_CMD + " " + txpowerdBm)
            else:
                print "Invalid tx power: " + str(txpowerdBm)
                print "valid values are: " + str(validvalues) + " dBm"
    
    def loopbackTest(self, lengthKB):
        
        if( self.BLEBoard ):
                        
            totalLength = int(lengthKB) * 1024       
            sequenceSent = generatePRBS("random", totalLength)        
            sequenceReceived = []
                    
            if (self.BLEBoard.isConnected == False):
                print "Board is not connected"
                return
                        
            try:
                
                print "Starting loopback test on UUT"
                print self.writeCommand(self.SMARTHALO_LOOPBACKTEST_UART_START_CMD, 1)
                
                self.BLEBoard.clearBuffers()
                
                chartime = time.time()                
                                                    
                self.BLEBoard.serialObj.write("".join(chr(x) for x in sequenceSent))                     
                print "Sent " + ",".join(hex(x) for x in sequenceSent)
                
                while( (len(sequenceSent) != len(sequenceReceived)) and ((1000*(time.time()-chartime)) < 1000)):
                
                    # Wait for bytes or timeout
                    while(self.BLEBoard.serialObj.in_waiting < 1 and ((1000*(time.time()-chartime)) < 1000)):
                        pass
                    
                    if self.BLEBoard.serialObj.in_waiting > 0:
                        chartime = time.time()
                        
                        line = self.BLEBoard.serialObj.read(self.BLEBoard.serialObj.in_waiting)                                           
                        print "Received " + ",".join(hex(ord(x)) for x in line)
            
                        sequenceReceived.extend(line)         
                    
                if len(sequenceSent) != len(sequenceReceived):
                    print "Received " + str(len(sequenceReceived)) + " bytes from " + str(len(sequenceSent)) + " bytes sent"
                    return -1
                
                # Count errors
                errors = 0
                
                for i in range(totalLength):
                
                    if chr(sequenceSent[i]) != sequenceReceived[i]:
                        
                        errors += 1
            
                return errors
            
            finally:
                
                print "Stopping loopback test on UUT"
                print self.writeCommand(self.SMARTHALO_LOOPBACKTEST_UART_STOP_CMD, 0)
                
                pass
        
        
class CentralDevice(BLEDevice):
    """
        Class to wrap everything related to a device used as
        an adapter (wire) to connect to a remote UUT device
    """
    SMARTHALO_USBBLEBRIDGE_ID_STRING = "USB-BLE Bridge"
    
    # Remote UUT commands
    SMARTHALO_REMOTE_CMD_SEP = "\\r"
    SMARTHALO_REMOTE_DATA_EOL = '\\n'
    SMARTHALO_RSSI_CMD = "GETRSSIDBM"
    SMARTHALO_TXPOWER_CMD = "SETTXPOWERDBM"
    SMARTHALO_GETACCELERATION_CMD = "GETACCELERATIONXYZ"
    SMARTHALO_GETMAGNETOMETER_CMD = "GETMAGNETOMETERXYZ"
    SMARTHALO_GETHEADING_CMD = "GETHEADING"
    SMARTHALO_LOOPBACKTEST_BLE_START_CMD = "STARTLOOPBACKTEST BLE"         
    
    # Central device commands
    SMARTHALO_BLE_SCANSTART_CMD = "SCANSTART"
    SMARTHALO_BLE_SCANSTOP_CMD = "SCANSTOP"
    SMARTHALO_DISCONNECT_BLE_CMD = b'\x1b' + "STOP"
    SMARTHALO_LOCAL_RSSI_CMD = b'\x1b' + SMARTHALO_RSSI_CMD     
    
    def __init__(self):        
        super(CentralDevice,self).__init__()
        super(CentralDevice,self).getBoard()
        self.getBoard()        
        self.BLEConnected = False
        
        if(self.connectBLE() == False):
            print "Could not establish a BLE connection to UUT"
    
    def __del__(self):
        
        if(self.BLEConnected):
            self.disconnectBLE()
    
    def getBoard(self):        
        
        skiplist = [];
        self.BLEBoard = PCA10040Board()
        currport = self.BLEBoard.findAndOpenPort(skiplist)
                       
        # Loop until we find a PCABoard running the test firmware or run out of ports   
        while currport != None:        

            print "Resetting device on port " + str(currport.device)
            print self.reset()
                            
            version = self.getVersion()
            
            if version != None:            
                print "version for PCA10040Board (CD): " + version
                
                if self.SMARTHALO_USBBLEBRIDGE_ID_STRING in version:
                    self.DevicePresent = True
                    # Reserve the port
                    self.BLEBoard.reserve_port(self.BLEBoard.port)
                    break                    
                     
            if not self.DevicePresent:
                skiplist.append(currport)
                
                if self.BLEBoard.isConnected:
                    self.BLEBoard.disconnect()
                
                currport = self.BLEBoard.findAndOpenPort(skiplist)        
        
        # No PCA10040 boards running the USB-BLE Bridge firmware. Lets look for SmartHaloBoards      
        if not self.DevicePresent:
             
            # Check if we have a Smart Halo Board present
            skiplist = []; 
            self.BLEBoard = SmartHaloBoard()
            currport = self.BLEBoard.findAndOpenPort(skiplist)
            
            # Loop until we find a SmartHaloBoard running the test firmware or run out of ports  
            while currport != None:
                
                print "Resetting device on port " + str(currport.device)
                print self.reset()
                
                version = self.getVersion()        
            
                if version != None:
                    print "version for SmartHaloBoard (CD): " + version
                    
                    if self.SMARTHALO_USBBLEBRIDGE_ID_STRING in version:
                        self.DevicePresent = True 
                        # Reserve the port
                        self.BLEBoard.reserve_port(self.BLEBoard.port)
                        break
                    
                if not self.DevicePresent:
                    skiplist.append(currport)
                    
                    if self.BLEBoard.isConnected:
                        self.BLEBoard.disconnect()
                    
                    currport = self.BLEBoard.findAndOpenPort(skiplist)
       
        return self.DevicePresent
            
    def connectBLE(self):
        
        if(self.DevicePresent):
                    
            lines = self.writeLocalCommand(self.SMARTHALO_BLE_SCANSTART_CMD,3)            
            print lines
            
            while("Bluetooth device connected" not in lines):
                print "Scanning for BLE devices"
                time.sleep(0.1)
                lines = self.BLEBoard.readLine('\n');
            
            print "BLE connection established"
            self.BLEConnected = True
                        
        return self.BLEConnected
            
    def disconnectBLE(self):
        
        if(self.DevicePresent and self.BLEConnected):
                    
            print "Disconnecting BLE device"
            
            print self.writeLocalCommand(self.SMARTHALO_DISCONNECT_BLE_CMD,3)
                        
            self.BLEConnected = False
            
            print "BLE device disconnected"            
                        
        return self.BLEConnected
        
    def writeLocalCommand(self, command, expectedresponselines=1):
        
        return super(CentralDevice,self).writeCommand(command, expectedresponselines)
    
    def writeRemoteCommand(self, command, expectedresponselines=1):
        
        if(self.BLEConnected):        
            return self.BLEBoard.writeCommand(command + self.SMARTHALO_REMOTE_CMD_SEP, expectedresponselines, self.SMARTHALO_REMOTE_DATA_EOL)
        
    def writeRemoteDataLine(self, line, expectedresponselines=1):
        
        if(self.BLEConnected):
            return self.BLEBoard.writeCommand(line, expectedresponselines, self.SMARTHALO_REMOTE_DATA_EOL)
        
    def testMagnetometer(self, sampleTimeSeconds):
        """ Prints magnetometer XYZ values and heading
            continuously until stopped
        """
    
        if(self.BLEConnected):
            
            while(True):
                            
                print self.writeRemoteCommand(self.SMARTHALO_GETMAGNETOMETER_CMD)        
                
                print self.writeRemoteCommand(self.SMARTHALO_GETHEADING_CMD)
                
                time.sleep(sampleTimeSeconds)

    def testAccelerometer(self, sampleTimeSeconds):
        """ Prints accelerometer XYZ values
            continuously until stopped
        """
        
        if(self.BLEConnected):
            
            while(True):
                print self.writeRemoteCommand(self.SMARTHALO_GETACCELERATION_CMD)
        
                time.sleep(sampleTimeSeconds)

    def getRSSI(self):
        """ Returns RSSI
        """
        if(self.BLEConnected):
        
            print self.writeLocalCommand(self.SMARTHALO_LOCAL_RSSI_CMD)

    def testRSSI(self, sampleTimeSeconds):
        """ Prints RSSI values continuously 
            until stopped
        """
        if(self.BLEConnected):
        
            while(True):
                print self.writeRemoteCommand(self.SMARTHALO_RSSI_CMD)
        
                time.sleep(sampleTimeSeconds)
                
    def setTxPower(self, txpowerdBm):
        """ Set the local TX power in dBm
            Valid values = -40, -30, -20, -16, -12, -8, -4, 0, 4
        """
        validvalues = [-40, -30, -20, -16, -12, -8, -4, 0, 4]
        
        if(self.DevicePresent):
            if (txpowerdBm in validvalues):
                print self.writeLocalCommand(self.SMARTHALO_TXPOWER_CMD + " " + txpowerdBm)
            else:
                print "Invalid tx power: " + str(txpowerdBm)
                print "valid values are: " + str(validvalues) + " dBm"
        
    def remoteloopbackTest(self, lengthKB):
                
        if( self.BLEBoard ):
                       
            totalLength = int(lengthKB) * 1024       
            sequenceSent = generatePRBS("counter", totalLength)        
            sequenceReceived = []
            
            if (self.BLEBoard.isConnected == False):
                print "Board is not connected"
                return
            
            try:
                
                print "Starting remote loopback test on UUT via central device"            
                print self.writeRemoteCommand(self.SMARTHALO_LOOPBACKTEST_BLE_START_CMD, 2)            
                                
                self.BLEBoard.clearBuffers()
                
                chartime = time.time()                
                                                    
                self.BLEBoard.serialObj.write("".join(chr(x) for x in sequenceSent))                     
                print "Sent " + "".join(chr(x) for x in sequenceSent)
                
                while( (len(sequenceSent) != len(sequenceReceived)) and ((1000*(time.time()-chartime)) < 5000)):
                
                    # Wait for bytes or timeout
                    while(self.BLEBoard.serialObj.in_waiting < 1 and ((1000*(time.time()-chartime)) < 5000)):
                        pass
                    
                    if self.BLEBoard.serialObj.in_waiting > 0:
                        chartime = time.time()
                        
                        line = self.BLEBoard.serialObj.read(self.BLEBoard.serialObj.in_waiting)                                           
                        print "Received " + line
            
                        sequenceReceived.extend(line)                                 
                    
                if len(sequenceSent) != len(sequenceReceived):
                    print "Received " + str(len(sequenceReceived)) + " bytes from " + str(len(sequenceSent)) + " bytes sent"
                    return -1
                                                        
                # Count errors
                errors = 0
                
                for i in range(totalLength):
                
                    if chr(sequenceSent[i]) != sequenceReceived[i]:
                        
                        errors += 1
            
                return errors
            
            finally:
                
                # HOW DO WE STOP THE LOOPBACK TEST??   
                    
                pass
            