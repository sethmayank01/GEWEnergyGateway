import serial
import time

PORT = "COM3"

ser = serial.Serial(
    PORT,
    baudrate=9600,
    bytesize=8,
    parity=serial.PARITY_EVEN,
    stopbits=1,
    timeout=0.1
)

print("Waiting for Modbus request...")

while True:

    req = ser.read(8)

    if len(req) == 8:

        print("RX :", " ".join(f"{b:02X}" for b in req))

        # Fake ABB reply
        response = bytes([
            0x01,
            0x03,
            0x24,

            0x43,0x71,0x99,0x9A,
            0x43,0x70,0x00,0x00,
            0x43,0x68,0x00,0x00,
            0x41,0x20,0x00,0x00,
            0x41,0x30,0x00,0x00,
            0x41,0x40,0x00,0x00,
            0x41,0x50,0x00,0x00,
            0x41,0x60,0x00,0x00,
            0x41,0x70,0x00,0x00,

            0x00,
            0x00          # CRC ignored for this test
        ])

        time.sleep(0.05)

        ser.write(response)

        print("TX :", " ".join(f"{b:02X}" for b in response))