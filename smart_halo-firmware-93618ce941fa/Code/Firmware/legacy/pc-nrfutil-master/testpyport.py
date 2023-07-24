import serial



ser = serial.Serial(
  port='COM3',
  baudrate=38400,
  parity=serial.PARITY_NONE,
  stopbits=serial.STOPBITS_ONE,
  bytesize=serial.EIGHTBITS,
  xonxoff=True,
  rtscts=False,
  dsrdtr=False
)
# ser.open()
ser.isOpen()

print(str(ser.baudrate))

print("Initializing the device ..")

ser.write(bytes([0xDC,0x00,0x00,0x03,0xB0,0x00,0x00,0x00,0xCD ]))