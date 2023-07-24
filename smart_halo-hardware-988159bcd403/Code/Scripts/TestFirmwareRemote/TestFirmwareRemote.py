import serial.serialutil

# My imports
from BLEDevice import UUTDevice, CentralDevice
from CommandLineInterface import *
import SerialBoard as SB

if __name__ == '__main__':
    
    myUUT = None
    myWire = None
    
    try:    
        #print "Available serial ports:"
        #SB.print_serial_ports()
    
        myUUT = UUTDevice()
        myWire = CentralDevice() 
        
        # This is to read the "Bluetooth device connected" message on UUT Uart interface
        if myUUT.BLEBoard.isConnected:
            while("Bluetooth device connected" not in myUUT.BLEBoard.readLine()):
                pass  
        
        # Make sure we have a UUT and a Central Device
        if myUUT.DevicePresent:
            print "UUT device found on " + myUUT.BLEBoard.boardID + " on port " + myUUT.BLEBoard.port.device
        else:        
            CommandLineInterface.program_exit(myUUT, myWire, "No UUT device found. Need both UUT and central device to control remotely a UUT device")
            
        if myWire.DevicePresent:
            print "Central device found on "  + myWire.BLEBoard.boardID + " on port " + myWire.BLEBoard.port.device
        else:        
            CommandLineInterface.program_exit(myUUT, myWire, "No central device found. Need both UUT and central device to control remotely a UUT device")
        
        # Establish the Bluetooth link
        if myWire.BLEConnected == False:
            CommandLineInterface.program_exit(myUUT, myWire, "Could not establish bluetooth connection between central device and UUT")
            
        # Assume we have both a UUT and a central device here.
        # Lets get a command        
             
        CommandLineInterface.print_help()        
        CommandLineInterface.print_prompt()
        
        while(True):
            
            inputline = raw_input()
            CommandLineInterface.command_parse_and_execute(myUUT, myWire, inputline)
    
    #except AttributeError as ae:
        
    #   print "Vachier attribut"
    #   raise ae
        
    except serial.serialutil.SerialException as se:
              
        print "Could not connect to serial device"
        raise se
        
    # Unhandled exception rethrow   
    except:
        
        raise
    
    finally:
        
        if myUUT != None:
            del myUUT
        if myWire != None:
            del myWire
    
        