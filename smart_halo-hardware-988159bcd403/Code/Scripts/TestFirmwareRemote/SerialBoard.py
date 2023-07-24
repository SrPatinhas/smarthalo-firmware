'''
Created on Jul 4, 2016

@author: sgelinas
'''

import serial.tools.list_ports
from abc import ABCMeta, abstractmethod

def print_serial_ports():
    """ Lists all serial ports main properties        
    """
    for port in serial.tools.list_ports.comports():
        print(port.device)        
        print(port.description)
        print(port.hwid)
        print(port.vid)
        print(port.pid)

class SerialBoard:
    """
        Base class that wraps everything common between
        all types of boards
    """
    
    __metaclass__ = ABCMeta
    available_ports = None
    
    def __init__(self, port=None, baudrate=None, rtscts=None, timeout=None, boardID=None):        
        self.isConnected = False        
        self.port = port
        self.baudrate = baudrate
        self.rtscts = rtscts
        self.timeout = timeout
        self.boardID = boardID
        
        try:
            self.serialObj = serial.Serial(self.port.device, baudrate=self.baudrate, rtscts=self.rtscts, timeout=self.timeout)        
            self.connect()
            
        # if unable to find port just continue. Will fail when accessing and isConnected = false
        except:
            pass
    
    def __del__(self):
        
        if( hasattr(self, 'isConnected')):
            if (self.isConnected == True):
                self.disconnect()
    
    def connect(self):
        
        if self.serialObj:        
            self.disconnect()
                
            try:
                print "Connecting to board " + str(self.boardID)
                self.serialObj.open()
            
            except:
                self.isConnected = False
                
            if self.serialObj.is_open:
                self.isConnected = True
                self.serialObj.reset_input_buffer()
                self.serialObj.reset_output_buffer()
                print "Connected to board " + str(self.boardID)
        
    def disconnect(self):
        
        if self.serialObj:        
            if self.serialObj.is_open:          
                self.serialObj.close()
                self.isConnected = False
       
    @abstractmethod     
    def findAndOpenPort(self,skiplist=[]):       
        if SerialBoard.available_ports == None:
            print "Populating the com port list"
            print_serial_ports()            
            SerialBoard.available_ports = serial.tools.list_ports.comports()
        
        return SerialBoard.available_ports
    
    def reserve_port(self, port):
        """ Takes off a reserved port off the list of
        available ports
        """
        
        if port in SerialBoard.available_ports:
            SerialBoard.available_ports.remove(port)
        else:
            print "Port was not found in available_ports list"
        
    def writeCommand(self, command, expectedresponselines=1, eol='\n'):
        
        # Write command
        self.writeLine(command, eol)
        
        lines = ""
        
        # Wait for response
        while(expectedresponselines > 0):
            lines += self.readLine('\n')
            expectedresponselines -= 1
            
        return lines          
    
    def writeLine(self, line, eol='\n'):
        
        if self.isConnected:            
            self.serialObj.write(line)
            self.serialObj.write(eol)
            try:
                print "Sent " + line + eol
            except TypeError:
                print "Sent " + str(line) + eol
        
    def readLine(self, eol='\n'):
                
        line = ""
        
        if self.isConnected:
            
            # Wait for incoming data
            while( self.serialObj.in_waiting <= 0 ):
                pass
            
            # Read until eol character
            c = self.serialObj.read(1)
            line += c            
            
            while( c != eol ):
                c = self.serialObj.read(1)
                line += c
                
            print "Receive " + line                
                                  
        return line          
    
    def clearBuffers(self):
    
        #self.serialObj.reset_input_buffer()
        #self.serialObj.reset_output_buffer()
        
        while(self.serialObj.in_waiting > 0):
            self.serialObj.read(1)

class SmartHaloBoard(SerialBoard):
    """ 
        Class to wrap everything specific to SmartHalo boards
    """
    SMARTHALO_EE_USB_ID_STRING = "USB Serial Port"
    SMARTHALO_BAUDRATE = 115200
    SMARTHALO_RTSCTS = True
    SMARTHALO_TIMEOUT = 0
    SMARTHALO_ID = "SmartHalo Board"
    SMARTHALO_VID_HEX_STR = "04B4"
    SMARTHALO_PID_HEX_STR = "0003"
    
    def findAndOpenPort(self,skiplist=[]):        
        for port in super(SmartHaloBoard,self).findAndOpenPort():
            # Cannot use VID/PID directly (not parsed correctly by pySerial)
            if port not in skiplist:
                if self.SMARTHALO_EE_USB_ID_STRING in port.description:
                    if self.SMARTHALO_VID_HEX_STR in port.hwid and self.SMARTHALO_PID_HEX_STR in port.hwid:
                        print "Found a SmartHalo board on port " + port.device 
                        self.port = port
                        super(SmartHaloBoard,self).__init__(self.port, self.SMARTHALO_BAUDRATE, self.SMARTHALO_RTSCTS, self.SMARTHALO_TIMEOUT, self.SMARTHALO_ID)           
                        return port
            
        return None

class PCA10040Board(SerialBoard):
    """
        Class to wrap everything specific to the PCA10040 board
    """
    PCA10040_USB_ID_STRING = "JLink CDC UART Port"
    PCA10040_BAUDRATE = 115200
    PCA10040_RTSCTS = True
    PCA10040_TIMEOUT = 0
    PCA10040_ID = "PCA10040 Board"
    PCA10040_VID = 4966
    PCA10040_PID = 4117
        
    def findAndOpenPort(self,skiplist=[]):        
        for port in super(PCA10040Board,self).findAndOpenPort():
            if port not in skiplist:            
                if self.PCA10040_USB_ID_STRING in port.description:
                    if port.vid == self.PCA10040_VID and port.pid == self.PCA10040_PID:
                        print "Found a PCA10040 board on port " + port.device  
                        self.port = port
                        super(PCA10040Board,self).__init__(self.port, self.PCA10040_BAUDRATE, self.PCA10040_RTSCTS, self.PCA10040_TIMEOUT, self.PCA10040_ID)           
                        return port
            
        return None
  
    