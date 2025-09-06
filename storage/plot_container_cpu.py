import signal
import sys
import pandas as pd
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
from tkinter import ttk

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

        # Scrollbar
        self.scrollbar = tk.Scale(self.control_frame, from_=self.xmin, to=self.xmax-self.window, orient=tk.HORIZONTAL,
                                  resolution=0.1, length=400, command=self.scroll)
        self.scrollbar.set(self.xmin)
        self.scrollbar.pack(side=tk.LEFT, padx=10)
        tk.Label(self.control_frame, text="Scroll X axis").pack(side=tk.LEFT)

        # Matplotlib Figure
        self.fig, self.ax = plt.subplots(figsize=(10, 5))
        self.canvas = FigureCanvasTkAgg(self.fig, master=master)
        self.canvas.get_tk_widget().pack(side=tk.RIGHT, fill=tk.BOTH, expand=1)

        self.lines = {}
        self.update_plot()

    def update_plot(self):
        self.ax.clear()
        xmin = self.scrollbar.get()
        xmax = xmin + self.window
        for c in containers:
            if self.selected[c].get():
                group = df[df['container_name'] == c]
                mask = (group['time'] >= xmin) & (group['time'] <= xmax)
                line, = self.ax.plot(group['time'][mask], group['cpu_usage'][mask], label=c)
                self.lines[c] = line
        self.ax.set_xlim(xmin, xmax)
        self.ax.set_xlabel('Time (seconds)')
        self.ax.set_ylabel('CPU Usage (%)')
        self.ax.set_title('Container CPU Usage Over Time')
        self.ax.legend()
        self.ax.grid(True)
        self.canvas.draw()

    def zoom_in(self):
        self.window = max((self.xmax - self.xmin) / 50, self.window / 2)
        self.update_plot()

    def zoom_out(self):
        self.window = min(self.xmax - self.xmin, self.window * 2)
        self.update_plot()

    def scroll(self, val):
        self.update_plot()

if __name__ == "__main__":
    root = tk.Tk()
    app = PlotApp(root)

    def handler(sig, frame):
        root.destroy()
        sys.exit(0)

    signal.signal(signal.SIGINT, handler)
    root.mainloop()