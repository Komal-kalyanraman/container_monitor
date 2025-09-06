import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.widgets import CheckButtons
import mplcursors

# Load CSV
df = pd.read_csv('container_metrics.csv')
df['time'] = (df['timestamp'] - df['timestamp'].min()) / 1000.0

containers = sorted(df['container_name'].unique())
fig, ax = plt.subplots(figsize=(12, 6))

lines = []
for container in containers:
    group = df[df['container_name'] == container]
    line, = ax.plot(group['time'], group['cpu_usage'], label=container, visible=True)
    lines.append(line)

ax.set_xlabel('Time (seconds)')
ax.set_ylabel('CPU Usage (%)')
ax.set_title('Container CPU Usage Over Time')
ax.legend()
ax.grid(True)
plt.tight_layout()

# Add interactive checkboxes
rax = plt.axes([0.02, 0.4, 0.13, 0.2])  # [left, bottom, width, height]
labels = containers
visibility = [True] * len(containers)
check = CheckButtons(rax, labels, visibility)

def func(label):
    idx = containers.index(label)
    lines[idx].set_visible(not lines[idx].get_visible())
    plt.draw()

check.on_clicked(func)

# Add interactive cursor
cursor = mplcursors.cursor(lines, hover=True)
@cursor.connect("add")
def on_add(sel):
    x, y = sel.target
    idx = lines.index(sel.artist)
    container = containers[idx]
    sel.annotation.set_text(f"Container: {container}\nTime: {x:.2f}\nCPU: {y:.2f}")

plt.show()