# =========================
# rpi_bridge.py
# =========================
#!/usr/bin/env python3
import socket
import struct
import threading
import time
import serial

# --- UDP ---
UDP_LISTEN_IP = "0.0.0.0"
UDP_CMD_PORT = 5005          # PC -> RPi (stav klaves)
UDP_TELEM_PORT = 5006        # RPi -> PC (telemetry)

# --- UART ---
UART_PORT = "/dev/serial0"   # /dev/serial0 alebo /dev/ttyAMA0 alebo /dev/ttyS0
UART_BAUD = 115200

# --- Protokol ---
START = 0xAB

# PC->STM32 (keys): [0xAB, state(0..63), state^0xFF]
def build_keys_frame(state6: int) -> bytes:
    s = state6 & 0x3F
    chk = (s ^ 0xFF) & 0xFF
    return bytes([START, s, chk])

# STM32->RPi:
#  - OLD distance: [0xAB, 0xD1, float32 LE, xor_chk(type+payload)]
#  - NEW frames:   [0xAB, TYPE, LEN, PAYLOAD..., xor_chk(TYPE+LEN+PAYLOAD)]
MSG_DISTANCE = 0xD1
MSG_TELEM8   = 0xD2   # 8x float32: ax,ay,az,gx,gy,gz,temp,dist

def xor_checksum(data: bytes) -> int:
    c = 0
    for b in data:
        c ^= b
    return c & 0xFF

class Shared:
    def __init__(self):
        self.last_pc_addr = None      # (ip, port) odkial prisiel posledny UDP cmd
        self.last_cmd_ts = 0.0
        self.last_state = 0

def uart_reader(ser: serial.Serial, udp_telem: socket.socket, shared: Shared):
    def read_exact(n: int) -> bytes:
        buf = b""
        while len(buf) < n:
            chunk = ser.read(n - len(buf))
            if not chunk:
                return b""
            buf += chunk
        return buf

    ok = 0
    bad = 0

    while True:
        b = ser.read(1)
        if not b:
            continue
        if b[0] != START:
            continue

        t = read_exact(1)
        if not t:
            continue
        msg_type = t[0]

        # --- OLD distance frame (bez LEN) ---
        if msg_type == MSG_DISTANCE:
            payload = read_exact(4)
            chk = read_exact(1)
            if not payload or not chk:
                continue

            calc = xor_checksum(bytes([msg_type]) + payload)
            if chk[0] != calc:
                bad += 1
                print(f"[UART BAD] D1 chk got={chk[0]:02X} calc={calc:02X} bad={bad}")
                continue

            dist_val = struct.unpack("<f", payload)[0]
            ok += 1
            print(f"[UART OK #{ok}] dist={dist_val:.3f}")

            if shared.last_pc_addr:
                udp_telem.sendto(f"dist={dist_val:.3f}".encode("utf-8"),
                                 (shared.last_pc_addr[0], UDP_TELEM_PORT))
            continue

        # --- NEW frames s LEN ---
        ln_b = read_exact(1)
        if not ln_b:
            continue
        ln = ln_b[0]

        payload = read_exact(ln)
        chk = read_exact(1)
        if not payload or not chk:
            continue

        calc = xor_checksum(bytes([msg_type, ln]) + payload)
        if chk[0] != calc:
            bad += 1
            print(f"[UART BAD] typ={msg_type:02X} ln={ln} chk got={chk[0]:02X} calc={calc:02X} bad={bad}")
            continue

        # Telemetry: 8 floats
        if msg_type == MSG_TELEM8 and ln == 32:
            ax, ay, az, gx, gy, gz, temp, dist = struct.unpack("<8f", payload)

            line = (f"ax={ax:.3f},ay={ay:.3f},az={az:.3f},"
                    f"gx={gx:.3f},gy={gy:.3f},gz={gz:.3f},"
                    f"temp={temp:.2f},dist={dist:.2f}")
            ok += 1
            print(f"[UART OK #{ok}] {line}")

            if shared.last_pc_addr:
                udp_telem.sendto(line.encode("utf-8"),
                                 (shared.last_pc_addr[0], UDP_TELEM_PORT))
        else:
            print(f"[UART SKIP] typ={msg_type:02X} ln={ln}")

def failsafe_loop(ser: serial.Serial, shared: Shared, timeout_s: float = 0.5):
    """Ak neprídu príkazy dlhšie než timeout_s, pošli STOP (0)."""
    while True:
        time.sleep(0.05)
        age = time.time() - shared.last_cmd_ts
        if shared.last_cmd_ts > 0 and age > timeout_s:
            if shared.last_state != 0:
                shared.last_state = 0
                frame = build_keys_frame(0)
                try:
                    ser.write(frame)
                    ser.flush()
                    print("[FAILSAFE] no UDP cmd -> STOP sent to STM32")
                except Exception as e:
                    print(f"[FAILSAFE] UART error: {e}")

def main():
    udp_cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_cmd.bind((UDP_LISTEN_IP, UDP_CMD_PORT))
    print(f"[UDP] listening on {UDP_LISTEN_IP}:{UDP_CMD_PORT}")

    udp_telem = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    ser = serial.Serial(UART_PORT, UART_BAUD, timeout=0.2)
    print(f"[UART] open {UART_PORT} @ {UART_BAUD}")

    shared = Shared()

    threading.Thread(target=uart_reader, args=(ser, udp_telem, shared), daemon=True).start()
    threading.Thread(target=failsafe_loop, args=(ser, shared), daemon=True).start()

    while True:
        data, addr = udp_cmd.recvfrom(64)
        if not data:
            continue

        shared.last_pc_addr = addr
        shared.last_cmd_ts = time.time()

        state = data[0] & 0x3F

        # pošli do STM32 len pri zmene (šetri UART)
        if state != shared.last_state:
            shared.last_state = state
            frame = build_keys_frame(state)
            ser.write(frame)
            ser.flush()
            print(f"[UDP->UART] pc={addr[0]} state={state:06b} frame={frame.hex(' ')}")

if __name__ == "__main__":
    main()