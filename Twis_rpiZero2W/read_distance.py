import serial
import struct
import time

START = 0xAB
MSG_DISTANCE = 0xD1

def xor_checksum(data: bytes) -> int:
    c = 0
    for b in data:
        c ^= b
    return c

def read_exact(ser: serial.Serial, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = ser.read(n - len(buf))
        if not chunk:
            # timeout
            return b""
        buf += chunk
    return buf

def main():   
    port = "/dev/serial0"
    baud = 115200  

    ser = serial.Serial(port, baudrate=baud, timeout=0.5)
    print(f"Listening on {port} @ {baud}...")

    good = 0
    bad = 0

    while True:
        b = ser.read(1)
        if not b:
            continue

        if b[0] != START:
            continue

        header = read_exact(ser, 1) 
        if not header:
            continue
        msg_type = header[0]

        if msg_type != MSG_DISTANCE:
            
            continue

        payload = read_exact(ser, 4)  
        if not payload:
            continue

        chk = read_exact(ser, 1)
        if not chk:
            continue

        calc = xor_checksum(bytes([msg_type]) + payload)
        if chk[0] != calc:
            bad += 1
            print(f"[BAD] checksum (got {chk[0]:02X}, calc {calc:02X}) bad={bad}")
            continue

        dist_m = struct.unpack("<f", payload)[0] 
        good += 1
        print(f"[OK #{good}] distance_m = {dist_m:.3f}")
        

if __name__ == "__main__":
    main()
