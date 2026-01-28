import socket
import threading
import queue
import time
import tkinter as tk
from tkinter import ttk

# === Nastav si IP RPi (tailscale / LAN) ===
RPI_IP = "100.127.162.111"
CMD_PORT = 5005       # posielanie state -> RPi
TEL_PORT = 5006       # prijímanie telemetry <- RPi

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
        self.title("TWIS Control + Distance")
        self.geometry("520x520")
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
        t = threading.Thread(target=self._telemetry_loop, daemon=True)
        t.start()

        # GUI update loop
        self.after(50, self._ui_tick)

        # HEARTBEAT: keep-alive posielanie stavu pravidelne
        self.after(HEARTBEAT_MS, self._heartbeat)

    def _build_ui(self):
        title = ttk.Label(self, text="Ovládanie (W/A/S/D/SPACE/E)", font=("Segoe UI", 14, "bold"))
        title.pack(anchor="w")

        desc = ttk.Label(
            self,
            text="Ovládanie funguje rovnako ako predtým.\n"
                 "Z STM32 sa posiela iba 'dist=xx.xx' a tu sa zobrazuje.\n"
                 f"Keep-alive: posielam stav každých {HEARTBEAT_MS} ms (kvôli RPi failsafe).",
            justify="left"
        )
        desc.pack(anchor="w", pady=(6, 12))

        # Frame pre tlačidlá
        btn_frame = ttk.Frame(self)
        btn_frame.pack(anchor="w", pady=(0, 14))

        self.buttons = {}

        def make_btn(text, bit):
            b = tk.Button(btn_frame, text=text, width=8, relief="flat", bd=0,
                          font=("Segoe UI", 11), padx=8, pady=8,
                          bg="#2b2b2b", fg="white", activebackground="#3a3a3a",
                          activeforeground="white")
            b.grid(padx=6, pady=6)
            b.bind("<ButtonPress-1>", lambda e: self._set_bit(bit, True))
            b.bind("<ButtonRelease-1>", lambda e: self._set_bit(bit, False))
            self.buttons[bit] = b
            return b

        make_btn("W", BIT_W).grid(row=0, column=1)
        make_btn("E", BIT_E).grid(row=0, column=3)

        make_btn("A", BIT_A).grid(row=1, column=0)
        make_btn("S", BIT_S).grid(row=1, column=1)
        make_btn("D", BIT_D).grid(row=1, column=2)
        make_btn("SPACE", BIT_SPACE).grid(row=1, column=3)

        # Telemetry panel
        tel_title = ttk.Label(self, text="Distance", font=("Segoe UI", 12, "bold"))
        tel_title.pack(anchor="w")

        self.status_var = tk.StringVar(value="Status: čakám na distance…")
        self.status_lbl = ttk.Label(self, textvariable=self.status_var)
        self.status_lbl.pack(anchor="w", pady=(4, 8))

        grid = ttk.Frame(self)
        grid.pack(anchor="w")

        self.dist = tk.StringVar(value="—")
        self._kv(grid, 0, "Dist:", self.dist)

        # debug raw
        self.raw = tk.Text(self, height=6, width=60, bg="#141414", fg="#cfcfcf", bd=0)
        self.raw.pack(fill="x", pady=(10, 0))
        self.raw.insert("end", "RAW telemetry log…\n")
        self.raw.configure(state="disabled")

    def _kv(self, parent, row, k, var):
        ttk.Label(parent, text=k, width=10).grid(row=row, column=0, sticky="w", pady=2)
        ttk.Label(parent, textvariable=var, width=18).grid(row=row, column=1, sticky="w", pady=2)

    def _bind_keys(self):
        # aby okno chytalo klávesy (stále treba mať focus)
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
        # keep-alive: pošli aj keď sa nezmenilo
        s = self.state & 0x3F
        self.sock_cmd.sendto(bytes([s]), (RPI_IP, CMD_PORT))
        self.last_sent = s

    def _heartbeat(self):
        # pravidelne posielaj stav (kvôli RPi failsafe)
        try:
            self._send_state_force()
        except OSError:
            return
        self.after(HEARTBEAT_MS, self._heartbeat)

    def _on_focus_out(self, _event=None):
        # keď okno stratí focus, radšej STOP, aby sa nezaseklo tlačidlo
        if self.state != 0:
            self.state = 0
            self._update_button_styles()
            self._send_state_force()

    def _on_close(self):
        # pošli STOP a zavri
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
                if not data:
                    continue
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

            try:
                line = data.decode("utf-8", errors="replace").strip()
            except Exception:
                line = repr(data)

            self._append_raw(line)

            parsed = self._parse_kv_line(line)

            # zobrazuj len distance
            if "dist" in parsed:
                self.dist.set(parsed["dist"])

        age = time.time() - self.last_tel_ts
        if self.last_tel_ts == 0:
            self.status_var.set("Status: čakám na distance…")
        elif age < 1.0:
            self.status_var.set("Status: distance OK")
        else:
            self.status_var.set(f"Status: bez distance {age:.1f}s")

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

    app = App()
    app.mainloop()
