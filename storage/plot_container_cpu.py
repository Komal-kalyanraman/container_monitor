import sys
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk

# Load CSV
df = pd.read_csv('container_metrics.csv')
df['time'] = (df['timestamp'] - df['timestamp'].min()) / 1000.0
containers = sorted(df['container_name'].unique())

class PlotApp:
    def __init__(self, master):
        self.master = master
        master.title("Container CPU Usage Dashboard")

        # Initial plot range
        self.xmin = df['time'].min()
        self.xmax = df['time'].max()
        self.window = (self.xmax - self.xmin) / 5  # Initial window size

        # Container selection
        self.selected = {c: tk.BooleanVar(value=True) for c in containers}
        self.check_frame = tk.LabelFrame(master, text="Containers")
        self.check_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)
        for c in containers:
            tk.Checkbutton(self.check_frame, text=c, variable=self.selected[c], command=self.update_plot).pack(anchor='w')

        # Controls
        self.control_frame = tk.Frame(master)
        self.control_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        tk.Button(self.control_frame, text="Zoom In", command=self.zoom_in).pack(side=tk.LEFT)
        tk.Button(self.control_frame, text="Zoom Out", command=self.zoom_out).pack(side=tk.LEFT)
        tk.Button(self.control_frame, text="Quit", command=self.quit_app, bg="red", fg="white").pack(side=tk.LEFT, padx=10)

        # Scrollbar
        self.scrollbar = tk.Scale(self.control_frame, from_=self.xmin, to=self.xmax-self.window, orient=tk.HORIZONTAL,
                                  resolution=0.1, length=400, command=self.scroll)
        self.scrollbar.set(self.xmin)
        self.scrollbar.pack(side=tk.LEFT, padx=10)
        tk.Label(self.control_frame, text="Scroll X axis").pack(side=tk.LEFT)

        # Main plot frame
        self.plot_frame = tk.Frame(master)
        self.plot_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=1)

        # Matplotlib Figure
        self.fig, self.ax = plt.subplots(figsize=(10, 5))
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Info label frame below plot
        self.label_frame = tk.Frame(self.plot_frame)
        self.label_frame.pack(side=tk.BOTTOM, fill=tk.X)
        self.info_label = tk.Label(
            self.label_frame,
            text="",
            font=("Courier New", 12),
            anchor="w",
            justify="left",
            width=50,
            height=6,
            bg="white",
            relief=tk.SUNKEN
        )
        self.info_label.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=10)

        self.lines = {}
        self.vline = None
        self.update_plot()

        # Mouse motion event
        self.canvas.mpl_connect("motion_notify_event", self.on_mouse_move)

        # Bind Ctrl+C to quit
        master.bind('<Control-c>', lambda event: self.quit_app())

    def update_plot(self):
        self.ax.clear()
        xmin = self.scrollbar.get()
        xmax = xmin + self.window
        self.lines = {}
        for c in containers:
            if self.selected[c].get():
                group = df[df['container_name'] == c]
                mask = (group['time'] >= xmin) & (group['time'] <= xmax)
                line, = self.ax.plot(group['time'][mask], group['cpu_usage'][mask], label=c)
                self.lines[c] = (line, group[mask])
        padding = 0.01 * (xmax - xmin)
        self.ax.set_xlim(xmin - padding, xmax + padding)
        self.ax.set_xlabel('Time (seconds)')
        self.ax.set_ylabel('CPU Usage (%)')
        self.ax.set_title('Container CPU Usage Over Time')
        self.ax.legend()
        self.ax.grid(True)
        # Draw vertical line (hidden initially)
        if self.vline is not None:
            self.vline.remove()
        self.vline = self.ax.axvline(x=xmin, color='gray', linestyle='dotted', linewidth=1, visible=False)
        self.canvas.draw()
        self.info_label.config(text="")

    def on_mouse_move(self, event):
        if event.inaxes == self.ax and event.xdata is not None:
            x = event.xdata
            # Move vertical line
            if self.vline is not None:
                self.vline.set_xdata(x)
                self.vline.set_visible(True)
            # Gather y values for all visible containers at x
            info_lines = [f"Time: {x:8.2f}"]
            for cname, (line, group) in self.lines.items():
                times = group['time'].values
                cpus = group['cpu_usage'].values
                if len(times) == 0:
                    continue
                idx = np.argmin(np.abs(times - x))
                info_lines.append(f"{cname:15}: CPU={cpus[idx]:8.2f}")
            # Pad/truncate to fixed number of lines
            while len(info_lines) < 6:
                info_lines.append("")
            info_text = "\n".join(info_lines[:6])
            self.info_label.config(text=info_text)
            self.canvas.draw_idle()
        else:
            if self.vline is not None:
                self.vline.set_visible(False)
            # Clear but keep label size
            self.info_label.config(text="\n" * 6)
            self.canvas.draw_idle()

    def zoom_in(self):
        self.window = max((self.xmax - self.xmin) / 50, self.window / 2)
        self.update_plot()

    def zoom_out(self):
        self.window = min(self.xmax - self.xmin, self.window * 2)
        self.update_plot()

    def scroll(self, val):
        self.update_plot()

    def quit_app(self):
        self.master.quit()
        self.master.destroy()
        sys.exit(0)

if __name__ == "__main__":
    root = tk.Tk()
    app = PlotApp(root)
    root.mainloop()