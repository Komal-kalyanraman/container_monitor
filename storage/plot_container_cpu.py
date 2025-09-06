import sys
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk

# Load CSVs
df = pd.read_csv('container_metrics.csv')
host_df = pd.read_csv('host_usage.csv')

# Compute time axis based on host_usage.csv
host_df['time'] = (host_df['timestamp'] - host_df['timestamp'].min()) / 1000.0
df['time'] = (df['timestamp'] - host_df['timestamp'].min()) / 1000.0  # Align container time to host time

containers = sorted(df['container_name'].unique())

class PlotApp:
    def __init__(self, master):
        self.master = master
        master.title("Container Resource Usage Dashboard")
        master.minsize(900, 700)

        # Initial plot range based on host_df
        self.xmin = host_df['time'].min()
        self.xmax = host_df['time'].max()
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
        self.scrollbar = tk.Scale(self.control_frame, from_=self.xmin, to=self.xmax-self.window, orient=tk.HORIZONTAL,
                                  resolution=0.1, length=400, command=self.scroll)
        self.scrollbar.set(self.xmin)
        self.scrollbar.pack(side=tk.LEFT, padx=10)
        tk.Label(self.control_frame, text="Scroll X axis").pack(side=tk.LEFT)

        # Main plot frame (fixed height)
        self.plot_frame = tk.Frame(master, height=500)
        self.plot_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=1)

        # Create three subplots: CPU, Memory, PIDs (fixed figure size, reduced hspace)
        self.fig, (self.ax_cpu, self.ax_mem, self.ax_pid) = plt.subplots(3, 1, figsize=(10, 7), sharex=True)
        self.fig.subplots_adjust(hspace=0.15)
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Info label frame below plot with vertical scrollbar
        self.label_frame = tk.Frame(self.plot_frame)
        self.label_frame.pack(side=tk.BOTTOM, fill=tk.X)
        self.info_scrollbar = tk.Scrollbar(self.label_frame, orient=tk.VERTICAL)
        self.info_text = tk.Text(
            self.label_frame,
            font=("Courier New", 12),
            width=70,
            height=8,
            bg="white",
            relief=tk.SUNKEN,
            yscrollcommand=self.info_scrollbar.set
        )
        self.info_scrollbar.config(command=self.info_text.yview)
        self.info_text.pack(side=tk.LEFT, fill=tk.X, expand=1, padx=10, pady=10)
        self.info_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # For each plot: lines and vertical line
        self.lines_cpu = {}
        self.lines_mem = {}
        self.lines_pid = {}
        self.vline_cpu = None
        self.vline_mem = None
        self.vline_pid = None

        self.update_plot()

        # Mouse motion event
        self.canvas.mpl_connect("motion_notify_event", self.on_mouse_move)

        # Bind Ctrl+C to quit
        master.bind('<Control-c>', lambda event: self.quit_app())

    def update_plot(self):
        # Clear all axes
        self.ax_cpu.clear()
        self.ax_mem.clear()
        self.ax_pid.clear()
        xmin = self.scrollbar.get()
        xmax = xmin + self.window
        padding = 0.01 * (xmax - xmin)
        # Draw host usage lines (black, normal width)
        mask_host = (host_df['time'] >= xmin) & (host_df['time'] <= xmax)
        self.ax_cpu.plot(host_df['time'][mask_host], host_df['cpu_usage_percent'][mask_host], label='Host CPU', color='black')
        self.ax_mem.plot(host_df['time'][mask_host], host_df['memory_usage_percent'][mask_host], label='Host Memory', color='black')
        # Draw lines for each container
        self.lines_cpu = {}
        self.lines_mem = {}
        self.lines_pid = {}
        for c in containers:
            if self.selected[c].get():
                group = df[df['container_name'] == c]
                mask = (group['time'] >= xmin) & (group['time'] <= xmax)
                # CPU
                line_cpu, = self.ax_cpu.plot(group['time'][mask], group['cpu_usage'][mask], label=c)
                self.lines_cpu[c] = (line_cpu, group[mask])
                # Memory
                line_mem, = self.ax_mem.plot(group['time'][mask], group['memory_usage'][mask], label=c)
                self.lines_mem[c] = (line_mem, group[mask])
                # PIDs
                line_pid, = self.ax_pid.plot(group['time'][mask], group['pids'][mask], label=c)
                self.lines_pid[c] = (line_pid, group[mask])
        # Set axis labels and limits
        for ax in [self.ax_cpu, self.ax_mem, self.ax_pid]:
            ax.set_xlim(xmin - padding, xmax + padding)
            ax.grid(True)
        self.ax_cpu.set_ylabel('CPU Usage (%)')
        self.ax_cpu.set_title('Container & Host CPU Usage Over Time')
        self.ax_mem.set_ylabel('Memory Usage (%)')
        self.ax_mem.set_title('Container & Host Memory Usage Over Time')
        self.ax_pid.set_ylabel('PIDs')
        self.ax_pid.set_xlabel('Time (seconds)')
        self.ax_pid.set_title('Container PIDs Over Time')
        self.ax_pid.legend() # Legend only for PIDs
        # Draw vertical lines (hidden initially)
        for vline in [self.vline_cpu, self.vline_mem, self.vline_pid]:
            if vline is not None:
                vline.remove()
        self.vline_cpu = self.ax_cpu.axvline(x=xmin, color='gray', linestyle='dotted', linewidth=1, visible=False)
        self.vline_mem = self.ax_mem.axvline(x=xmin, color='gray', linestyle='dotted', linewidth=1, visible=False)
        self.vline_pid = self.ax_pid.axvline(x=xmin, color='gray', linestyle='dotted', linewidth=1, visible=False)
        self.canvas.draw()
        self.info_text.delete("1.0", tk.END)

    def on_mouse_move(self, event):
        # Only respond if mouse is in one of the axes and xdata is valid
        if event.inaxes in [self.ax_cpu, self.ax_mem, self.ax_pid] and event.xdata is not None:
            x = event.xdata
            # Move vertical lines in all plots
            for vline in [self.vline_cpu, self.vline_mem, self.vline_pid]:
                if vline is not None:
                    vline.set_xdata(x)
                    vline.set_visible(True)
            # Gather values for all visible containers at x
            info_lines = [f"Time: {x:8.2f}"]
            # Host info
            host_idx = np.argmin(np.abs(host_df['time'].values - x))
            host_cpu = host_df['cpu_usage_percent'].values[host_idx]
            host_mem = host_df['memory_usage_percent'].values[host_idx]
            info_lines.append(f"{'Host':15}: CPU={host_cpu:8.2f}  MEM={host_mem:8.2f}")
            for cname in containers:
                if self.selected[cname].get():
                    # CPU
                    group_cpu = self.lines_cpu.get(cname, (None, None))[1]
                    cpu_val = ""
                    if group_cpu is not None and len(group_cpu) > 0:
                        times = group_cpu['time'].values
                        cpus = group_cpu['cpu_usage'].values
                        idx = np.argmin(np.abs(times - x))
                        cpu_val = f"{cpus[idx]:8.2f}"
                    # Memory
                    group_mem = self.lines_mem.get(cname, (None, None))[1]
                    mem_val = ""
                    if group_mem is not None and len(group_mem) > 0:
                        times = group_mem['time'].values
                        mems = group_mem['memory_usage'].values
                        idx = np.argmin(np.abs(times - x))
                        mem_val = f"{mems[idx]:8.2f}"
                    # PIDs
                    group_pid = self.lines_pid.get(cname, (None, None))[1]
                    pid_val = ""
                    if group_pid is not None and len(group_pid) > 0:
                        times = group_pid['time'].values
                        pids = group_pid['pids'].values
                        idx = np.argmin(np.abs(times - x))
                        pid_val = f"{pids[idx]:8.0f}"
                    info_lines.append(f"{cname:15}: CPU={cpu_val}  MEM={mem_val}  PIDs={pid_val}")
            info_text = "\n".join(info_lines)
            self.info_text.delete("1.0", tk.END)
            self.info_text.insert(tk.END, info_text)
            self.canvas.draw_idle()
        else:
            for vline in [self.vline_cpu, self.vline_mem, self.vline_pid]:
                if vline is not None:
                    vline.set_visible(False)
            self.info_text.delete("1.0", tk.END)
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