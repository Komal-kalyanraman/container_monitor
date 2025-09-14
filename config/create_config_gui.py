import tkinter as tk
from tkinter import ttk, messagebox
import os

# Predefined limits and options
LIMITS = {
    "ui_refresh_interval_ms": (500, 3000),
    "resource_sampling_interval_ms": (10, 1000),
    "container_event_refresh_interval_ms": (10, 1000),
    "batch_size": (1, 100),
    "alert_warning": (0.0, 100.0),
    "alert_critical": (0.0, 100.0),
    "thread_count": (1, 10),
    "thread_capacity": (1, 10),
}
OPTIONS = {
    "runtime": ["docker", "podman"],
    "cgroup": ["v1", "v2"],
    "database": ["sqlite", "mysql"],
    "ui_enabled": ["true", "false"],
}
DEFAULTS = {
    "db_path": "../../storage/metrics.db",
    "file_export_folder_path": "../../storage"
}

FIELDS = [
    ("runtime", "OptionMenu"),
    ("cgroup", "OptionMenu"),
    ("database", "OptionMenu"),
    ("ui_refresh_interval_ms", "Spinbox"),
    ("resource_sampling_interval_ms", "Spinbox"),
    ("container_event_refresh_interval_ms", "Spinbox"),
    ("db_path", "Entry"),
    ("ui_enabled", "OptionMenu"),
    ("batch_size", "Spinbox"),
    ("alert_warning", "Spinbox"),
    ("alert_critical", "Spinbox"),
    ("thread_count", "Spinbox"),
    ("thread_capacity", "Spinbox"),
    ("file_export_folder_path", "Entry"),
]

def save_config(values):
    try:
        lines = []
        for key, value in values.items():
            lines.append(f"{key}={value}")
        with open("parameter.conf", "w") as f:
            f.write("\n".join(lines))
        messagebox.showinfo("Success", "Config file 'parameter.conf' created successfully!")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to write config file: {e}")

def validate(values):
    for key, value in values.items():
        if key in LIMITS:
            minv, maxv = LIMITS[key]
            try:
                v = float(value) if "." in value else int(value)
            except ValueError:
                return False, f"Invalid value for {key}"
            if not (minv <= v <= maxv):
                return False, f"{key} must be between {minv} and {maxv}"
        if key in OPTIONS and value not in OPTIONS[key]:
            return False, f"Invalid option for {key}"
    return True, ""

class ConfigApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Container Monitor Config Generator")
        self.resizable(False, False)
        self.entries = {}
        self.vars = {}

        main_frame = ttk.Frame(self, padding="12 12 12 12")
        main_frame.grid(row=0, column=0, sticky="nsew")
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        label_width = 28  # Minimum width for labels

        for idx, (key, widget_type) in enumerate(FIELDS):
            label = ttk.Label(main_frame, text=key + ":", anchor="e", width=label_width)
            label.grid(row=idx, column=0, sticky="e", padx=(0, 10), pady=4)
            if widget_type == "OptionMenu":
                var = tk.StringVar(value=OPTIONS[key][0])
                self.vars[key] = var
                cb = ttk.Combobox(main_frame, textvariable=var, values=OPTIONS[key], state="readonly", width=20)
                cb.current(0)
                cb.grid(row=idx, column=1, sticky="w", padx=(0, 10), pady=4)
            elif widget_type == "Spinbox":
                minv, maxv = LIMITS[key]
                var = tk.StringVar(value=str(minv))
                self.vars[key] = var
                sb = tk.Spinbox(main_frame, from_=minv, to=maxv, textvariable=var, increment=1 if isinstance(minv, int) else 0.1, width=22)
                sb.grid(row=idx, column=1, sticky="w", padx=(0, 10), pady=4)
            elif widget_type == "Entry":
                var = tk.StringVar(value=DEFAULTS.get(key, ""))
                self.vars[key] = var
                ent = ttk.Entry(main_frame, textvariable=var, width=24)
                ent.grid(row=idx, column=1, sticky="w", padx=(0, 10), pady=4)

        btn = ttk.Button(main_frame, text="Create Config", command=self.on_submit)
        btn.grid(row=len(FIELDS), column=0, columnspan=2, pady=16)

    def on_submit(self):
        values = {k: v.get() for k, v in self.vars.items()}
        valid, msg = validate(values)
        if valid:
            save_config(values)
        else:
            messagebox.showerror("Validation Error", msg)

if __name__ == "__main__":
    app = ConfigApp()
    app.mainloop()