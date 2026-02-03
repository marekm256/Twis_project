# =========================
# pc_gui.py
# =========================
import socket
import threading
import queue
import time
import tkinter as tk
from tkinter import ttk

# === Nastav si IP RPi (tailscale / LAN) ===
RPI_IP = "192.168.137.2"
CMD_PORT = 5005          # posielanie state -> RPi
TEL_PORT = 5006          # prijímanie telemetry <- RPi

# ako často posielať keep-alive (musí byť < 500ms kvôli RPi failsafe)
HEARTBEAT_MS = 50

BIT_W     = 1 << 0
BIT_A     = 1 << 1
BIT_S     = 1 << 2
BIT_D     = 1 << 3
BIT_SPACE = 1 << 4
BIT_E     = 1 << 5


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("TWIS Control + Telemetry")
        self.geometry("560x700")  # trochu vyššie kvôli novým poliam
        self.configure(padx=16, pady=16)

        # UDP socket na posielanie state
        self.sock_cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # UDP socket na prijímanie telemetry
        self.sock_tel = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock_tel.bind(("0.0.0.0", TEL_PORT))
        self.sock_tel.settimeout(0.5)

        self.state = 0
        self.last_sent = None
        self.last_tel_ts = 0.0

        self.tel_q = queue.Queue()

        self._build_ui()
        self._bind_keys()

        # keď stratíš focus (alt-tab, klik mimo), radšej STOP
        self.bind("<FocusOut>", self._on_focus_out)

        # pri zatvorení okna pošli STOP
        self.protocol("WM_DELETE_WINDOW", self._on_close)

        # background thread na telemetry
        threading.Thread(target=self._telemetry_loop, daemon=True).start()

        # GUI update loop
        self.after(50, self._ui_tick)

        # HEARTBEAT: keep-alive posielanie stavu pravidelne
        self.after(HEARTBEAT_MS, self._heartbeat)

    def _build_ui(self):
        title = ttk.Label(self, text="Ovládanie (W/A/S/D/SPACE/E)", font=("Segoe UI", 14, "bold"))
        title.pack(anchor="w")

        btn_frame = ttk.Frame(self)
        btn_frame.pack(anchor="w", pady=(8, 14))

        self.buttons = {}

        def make_btn(text, bit):
            b = tk.Button(btn_frame, text=text, width=8, relief="flat", bd=0,
                          font=("Segoe UI", 11), padx=8, pady=8,
                          bg="#2b2b2b", fg="white", activebackground="#3a3a3a",
                          activeforeground="white")
            b.bind("<ButtonPress-1>", lambda e: self._set_bit(bit, True))
            b.bind("<ButtonRelease-1>", lambda e: self._set_bit(bit, False))
            self.buttons[bit] = b
            return b

        make_btn("W", BIT_W).grid(row=0, column=1, padx=6, pady=6)
        make_btn("E", BIT_E).grid(row=0, column=3, padx=6, pady=6)

        make_btn("A", BIT_A).grid(row=1, column=0, padx=6, pady=6)
        make_btn("S", BIT_S).grid(row=1, column=1, padx=6, pady=6)
        make_btn("D", BIT_D).grid(row=1, column=2, padx=6, pady=6)
        make_btn("SPACE", BIT_SPACE).grid(row=1, column=3, padx=6, pady=6)

        tel_title = ttk.Label(self, text="Telemetry", font=("Segoe UI", 12, "bold"))
        tel_title.pack(anchor="w", pady=(10, 0))

        self.status_var = tk.StringVar(value="Status: čakám na telemetry…")
        ttk.Label(self, textvariable=self.status_var).pack(anchor="w", pady=(4, 8))

        grid = ttk.Frame(self)
        grid.pack(anchor="w")

        # existujúce
        self.dist = tk.StringVar(value="—")
        self.ax = tk.StringVar(value="—")
        self.ay = tk.StringVar(value="—")
        self.az = tk.StringVar(value="—")
        self.gx = tk.StringVar(value="—")
        self.gy = tk.StringVar(value="—")
        self.gz = tk.StringVar(value="—")
        self.temp = tk.StringVar(value="—")

        # nové
        self.roll = tk.StringVar(value="—")
        self.mean_roll = tk.StringVar(value="—")
        self.mean_dist = tk.StringVar(value="—")

        r = 0
        self._kv(grid, r, "Dist (cm):", self.dist); r += 1
        self._kv(grid, r, "MeanDist:", self.mean_dist); r += 1
        self._kv(grid, r, "Roll (deg):", self.roll); r += 1
        self._kv(grid, r, "MeanRoll:", self.mean_roll); r += 1
        r += 1  # malý odstup

        self._kv(grid, r, "Ax (g):", self.ax); r += 1
        self._kv(grid, r, "Ay (g):", self.ay); r += 1
        self._kv(grid, r, "Az (g):", self.az); r += 1
        self._kv(grid, r, "Gx (dps):", self.gx); r += 1
        self._kv(grid, r, "Gy (dps):", self.gy); r += 1
        self._kv(grid, r, "Gz (dps):", self.gz); r += 1
        self._kv(grid, r, "Temp (°C):", self.temp); r += 1

        self.raw = tk.Text(self, height=10, width=70, bg="#141414", fg="#cfcfcf", bd=0)
        self.raw.pack(fill="x", pady=(12, 0))
        self.raw.insert("end", "RAW telemetry log…\n")
        self.raw.configure(state="disabled")

    def _kv(self, parent, row, k, var):
        ttk.Label(parent, text=k, width=12).grid(row=row, column=0, sticky="w", pady=2)
        ttk.Label(parent, textvariable=var, width=18).grid(row=row, column=1, sticky="w", pady=2)

    def _bind_keys(self):
        self.focus_set()

        # press
        self.bind("<KeyPress-w>", lambda e: self._set_bit(BIT_W, True))
        self.bind("<KeyPress-a>", lambda e: self._set_bit(BIT_A, True))
        self.bind("<KeyPress-s>", lambda e: self._set_bit(BIT_S, True))
        self.bind("<KeyPress-d>", lambda e: self._set_bit(BIT_D, True))
        self.bind("<KeyPress-e>", lambda e: self._set_bit(BIT_E, True))
        self.bind("<KeyPress-space>", lambda e: self._set_bit(BIT_SPACE, True))

        # release
        self.bind("<KeyRelease-w>", lambda e: self._set_bit(BIT_W, False))
        self.bind("<KeyRelease-a>", lambda e: self._set_bit(BIT_A, False))
        self.bind("<KeyRelease-s>", lambda e: self._set_bit(BIT_S, False))
        self.bind("<KeyRelease-d>", lambda e: self._set_bit(BIT_D, False))
        self.bind("<KeyRelease-e>", lambda e: self._set_bit(BIT_E, False))
        self.bind("<KeyRelease-space>", lambda e: self._set_bit(BIT_SPACE, False))

    def _set_bit(self, bit, on: bool):
        if on:
            self.state |= bit
        else:
            self.state &= ~bit

        self._update_button_styles()
        self._send_state_if_changed()

    def _update_button_styles(self):
        for bit, btn in self.buttons.items():
            if self.state & bit:
                btn.configure(bg="#e53935", fg="white")
            else:
                btn.configure(bg="#2b2b2b", fg="white")

    def _send_state_if_changed(self):
        s = self.state & 0x3F
        if s != self.last_sent:
            self.sock_cmd.sendto(bytes([s]), (RPI_IP, CMD_PORT))
            self.last_sent = s

    def _send_state_force(self):
        s = self.state & 0x3F
        self.sock_cmd.sendto(bytes([s]), (RPI_IP, CMD_PORT))
        self.last_sent = s

    def _heartbeat(self):
        try:
            self._send_state_force()
        except OSError:
            return
        self.after(HEARTBEAT_MS, self._heartbeat)

    def _on_focus_out(self, _event=None):
        if self.state != 0:
            self.state = 0
            self._update_button_styles()
            self._send_state_force()

    def _on_close(self):
        try:
            self.state = 0
            self._send_state_force()
        except Exception:
            pass
        try:
            self.sock_tel.close()
        except Exception:
            pass
        try:
            self.sock_cmd.close()
        except Exception:
            pass
        self.destroy()

    def _telemetry_loop(self):
        while True:
            try:
                data, _ = self.sock_tel.recvfrom(2048)
                if data:
                    self.tel_q.put(data)
            except socket.timeout:
                pass
            except OSError:
                break

    def _ui_tick(self):
        while True:
            try:
                data = self.tel_q.get_nowait()
            except queue.Empty:
                break

            self.last_tel_ts = time.time()

            line = data.decode("utf-8", errors="replace").strip()
            self._append_raw(line)

            parsed = self._parse_kv_line(line)

            # doplnené nové kľúče: roll, mean_roll, mean_dist
            mapping = [
                ("dist", self.dist),
                ("mean_dist", self.mean_dist),
                ("roll", self.roll),
                ("mean_roll", self.mean_roll),

                ("ax", self.ax), ("ay", self.ay), ("az", self.az),
                ("gx", self.gx), ("gy", self.gy), ("gz", self.gz),
                ("temp", self.temp),
            ]
            for k, var in mapping:
                if k in parsed:
                    var.set(parsed[k])

        age = time.time() - self.last_tel_ts
        if self.last_tel_ts == 0:
            self.status_var.set("Status: čakám na telemetry…")
        elif age < 1.0:
            self.status_var.set("Status: telemetry OK")
        else:
            self.status_var.set(f"Status: bez telemetry {age:.1f}s")

        self.after(50, self._ui_tick)

    def _append_raw(self, line: str):
        self.raw.configure(state="normal")
        self.raw.insert("end", line + "\n")
        self.raw.see("end")
        self.raw.configure(state="disabled")

    @staticmethod
    def _parse_kv_line(line: str) -> dict:
        out = {}
        for part in line.split(","):
            part = part.strip()
            if "=" in part:
                k, v = part.split("=", 1)
                out[k.strip()] = v.strip()
        return out


if __name__ == "__main__":
    
    try:
        from ctypes import windll
        windll.shcore.SetProcessDpiAwareness(1)
    except Exception:
        pass

    App().mainloop()
