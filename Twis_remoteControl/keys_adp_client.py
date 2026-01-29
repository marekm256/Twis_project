import socket
from pynput import keyboard

# nastav IP RPi v tailscale (napr. 100.x.x.x) a port
RPI_IP = "100.127.162.111"
RPI_PORT = 5005

BIT_W = 1 << 0
BIT_A = 1 << 1
BIT_S = 1 << 2
BIT_D = 1 << 3
BIT_SPACE = 1 << 4
BIT_E = 1 << 5

state = 0
last_sent = None

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_state():
    global last_sent
    s = state & 0x3F
    if s != last_sent:
        sock.sendto(bytes([s]), (RPI_IP, RPI_PORT))
        last_sent = s
        print(f"sent state: {s:06b}")

def on_press(key):
    global state
    try:
        if key.char == 'w': state |= BIT_W
        elif key.char == 'a': state |= BIT_A
        elif key.char == 's': state |= BIT_S
        elif key.char == 'd': state |= BIT_D
        elif key.char == 'e': state |= BIT_E
        else: return
    except AttributeError:
        if key == keyboard.Key.space:
            state |= BIT_SPACE
        else:
            return
    send_state()

def on_release(key):
    global state
    try:
        if key.char == 'w': state &= ~BIT_W
        elif key.char == 'a': state &= ~BIT_A
        elif key.char == 's': state &= ~BIT_S
        elif key.char == 'd': state &= ~BIT_D
        elif key.char == 'e': state &= ~BIT_E
        else: return
    except AttributeError:
        if key == keyboard.Key.space:
            state &= ~BIT_SPACE
        else:
            return
    send_state()

    # esc ukončí
    if key == keyboard.Key.esc:
        return False

print("Running. Use W/A/S/D/SPACE/E. ESC to quit.")
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()