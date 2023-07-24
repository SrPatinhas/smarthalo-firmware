'''
Created on Jul 4, 2016

@author: sgelinas
'''

import sys
import time
import multiprocessing

class CommandLineInterface:
    
    @staticmethod
    def printmessage(message):
    
        sys.stdout.write(message)
        sys.stdout.flush()
    
    @staticmethod    
    def print_help():
        """ List available commands
        """
        print "Enter a command:"
        print "help:\t\t\t\t\tTo show help"
        print "quit:\t\t\t\t\tTo quit"
        print "remotetxpower <power>:\t\t\tSet the remote BLE tx power"
        print "remoterssi <sample time> <timeout>:\tTo continuously log rssi every sample time until timeout (seconds)"
        print "remoteloopback <length>:\t\tStarts a remote loopback test of length KB"
        print "accelero <sample time> <timeout>:\tTo continuously log accelero every sample time until timeout (seconds). 0=run indefinitely"
        print "magneto <sample time> <timeout>:\tTo continuously log magneto every sample time until timeout (seconds). 0=run indefinitely"                
        print "localtxpower <power>:\t\t\tSet the local BLE tx power"
        print "localrssi:\t\t\t\tPrints the BLE rssi at the local end"
        print "loopback <length>:\t\t\tStarts a loopback test of length KB"
    
    @staticmethod
    def print_prompt():
        CommandLineInterface.printmessage('TestFirmwareRemote>')
    
    @staticmethod
    def program_exit(uut, wire, reason):
        
        del uut, wire
        
        sys.exit(reason)
    
    @staticmethod
    def command_parse_and_execute(uut, wire, command):
        
        if command == 'quit':
            
            CommandLineInterface.program_exit(uut, wire, 'Have a good day :)')
        
        elif command == 'help':
            
            CommandLineInterface.print_help()
            
        elif 'remoterssi' in command:
             
            if len(command.split(" ")) == 1:
                print 'Missing sample time and timeout for remoterssi command'
            elif len(command.split(" ")) == 2:
                print 'Missing timeout for remoterssi command'
            else:
                sampleTime = float(command.split(" ")[1])
                timeout = float(command.split(" ")[2])
                p = multiprocessing.Process(target=wire.testRSSI, args=(sampleTime,))
                p.start()
                if timeout != 0:
                    p.join(timeout)
                else:
                    p.join() 
                         
        elif 'accelero' in command:
                             
            if len(command.split(" ")) == 1:
                print 'Missing sample time and timeout for accelero command'
            elif len(command.split(" ")) == 2:
                print 'Missing timeout for accelero command'
            else:
                sampleTime = float(command.split(" ")[1])
                timeout = float(command.split(" ")[2])
                p = multiprocessing.Process(target=wire.testAccelerometer, args=(sampleTime,))
                p.start()
                if timeout != 0:
                    p.join(timeout)
                else:
                    p.join() 
                    
        elif 'magneto' in command:      
                      
            if len(command.split(" ")) == 1:
                print 'Missing sample time and timeout for magneto command'
            elif len(command.split(" ")) == 2:
                print 'Missing timeout for magneto command'
            else:
                sampleTime = float(command.split(" ")[1])
                timeout = float(command.split(" ")[2])
                p = multiprocessing.Process(target=wire.testMagnetometer, args=(sampleTime,))
                p.start()
                if timeout != 0:
                    p.join(timeout)
                else:
                    p.join() 
                    
        elif 'localtxpower' in command:
                
            if len(command.split(" ")) == 1:
                print 'Missing power for localtxpower command'
            else:
                wire.setTxPower(float(command.split(" ")[1]))
                
        elif 'remoteloopback' in command:
            
            if len(command.split(" ")) == 1:
                print 'Missing PRBS sequence length for remote loopback command'
            else:
                lengthkB = int(command.split(" ")[1])
                start = time.time()                
                print str(wire.remoteloopbackTest(lengthkB)) + " errors"
                print "average throughput = %.1f kbps" % (((1024*lengthkB*8)/1000)/(time.time() - start)) 
                
        elif 'loopback' in command:
            
            if len(command.split(" ")) == 1:
                print 'Missing PRBS sequence length for loopback command'
            else:
                lengthkB = int(command.split(" ")[1])
                start = time.time()                
                print str(uut.loopbackTest(lengthkB)) + " errors"
                print "average throughput = %.1f kbps" % (((1024*lengthkB*8)/1000)/(time.time() - start))
                
        elif 'remotetxpower' in command:      
                         
            if len(command.split(" ")) == 1:
                print 'Missing power for remotetxpower command'
            else:
                uut.setTxPower(float(command.split(" ")[1]))
                
        elif command == 'localrssi':
            
            wire.getRSSI()
            
        elif command is '':
            
            pass
            
        else:
            print "Invalid Command \"" + command + "\""
            
        CommandLineInterface.print_prompt()     