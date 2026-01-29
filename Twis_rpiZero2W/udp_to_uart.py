import socket
import serial
import time

UART = "/dev/serial0"
BAUD = 115200

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

START = 0xAB

def make_frame(state: int) -> bytes:
    state &= 0x3F
    chk = state ^ 0xFF
    return bytes([START, state, chk])

def main():
    ser = serial.Serial(UART, BAUD, timeout=0)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    sock.settimeout(0.2)

    last_rx = time.monotonic()
    last_state = None

    print(f"Listening UDP {UDP_IP}:{UDP_PORT}, forwarding to UART {UART}@{BAUD}")

    while True:
        try:
            data, addr = sock.recvfrom(16)
            if not data:
                continue

            state = data[0] & 0x3F
            last_rx = time.monotonic()

            print(f"RX UDP from {addr[0]}  state = {state:06b}")

            if state != last_state:
                ser.write(make_frame(state))
                print(f" -> UART sent  state = {state:06b}")
                last_state = state

        except socket.timeout:
            pass

if __name__ == "__main__":
    main()