import subprocess
import os
import sys
import time
import json

def get_running_containers():
    cmd = ["sudo", "podman", "ps", "--format", "{{.ID}} {{.Names}}"]
    output = subprocess.check_output(cmd).decode().strip()
    containers = []
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            containers.append({"Id": parts[0], "Name": parts[1]})
    return containers

def get_pid(container_name):
    cmd = ["sudo", "podman", "inspect", "-f", "{{.State.Pid}}", container_name]
    try:
        pid = subprocess.check_output(cmd).decode().strip()
        return pid
    except Exception:
        return None

def detect_cgroup_version():
    if os.path.exists("/sys/fs/cgroup/cgroup.controllers"):
        return "v2"
    return "v1"

def get_cgroup_paths(pid, version):
    paths = {}
    try:
        with open(f"/proc/{pid}/cgroup") as f:
            for line in f:
                parts = line.strip().split(":")
                if version == "v2" and parts[0] == "0":
                    paths["base"] = parts[2]
                elif version == "v1":
                    if "memory" in parts[1]:
                        paths["memory"] = parts[2]
                    if "cpu" in parts[1]:
                        paths["cpu"] = parts[2]
                    if "pids" in parts[1]:
                        paths["pids"] = parts[2]
    except Exception:
        pass
    return paths

def safe_read(path):
    try:
        with open(path) as f:
            return f.read().strip()
    except Exception:
        return "N/A"

def get_stats_v1(paths, mem_limit_bytes):
    membase = f"/sys/fs/cgroup/memory{paths.get('memory','')}"
    mem_usage = safe_read(f"{membase}/memory.usage_in_bytes")
    mem_usage = int(mem_usage) if mem_usage != "N/A" else 0
    mem_limit = safe_read(f"{membase}/memory.limit_in_bytes")
    mem_limit = int(mem_limit) if mem_limit != "N/A" else mem_limit_bytes
    mem_percent = (mem_usage / mem_limit * 100) if mem_limit and mem_limit != "N/A" else 0

    cpubase = f"/sys/fs/cgroup/cpu{paths.get('cpu','')}"
    cpu_usage = safe_read(f"{cpubase}/cpuacct.usage")
    cpu_usage = int(cpu_usage) if cpu_usage != "N/A" else 0

    pidbase = f"/sys/fs/cgroup/pids{paths.get('pids','')}"
    pids_current = safe_read(f"{pidbase}/pids.current")
    pids_max = safe_read(f"{pidbase}/pids.max")
    pids_current = int(pids_current) if pids_current != "N/A" else 0
    pids_max = int(pids_max) if pids_max not in ["N/A", "max"] else 0

    return mem_usage, mem_limit, mem_percent, cpu_usage, pids_current, pids_max

def format_stats(container_id, name, cpu_pct, mem_usage, mem_limit, mem_percent, pids_current, avg_cpu_pct):
    mem_usage_mb = mem_usage / 1024 / 1024
    mem_limit_mb = mem_limit / 1024 / 1024 if mem_limit else 0
    return f"{container_id[:12]:<12} {name:<14} {cpu_pct:>7.2f}%   {mem_usage_mb:>7.2f}MB / {mem_limit_mb:.1f}MB   {mem_percent:>6.2f}%   {pids_current:>5}   {avg_cpu_pct:>7.2f}%"

class ContainerMonitor:
    def __init__(self, version):
        self.version = version
        self.containers = {}  # id -> info dict

    def update_containers(self):
        running = get_running_containers()
        running_ids = set(c["Id"] for c in running)
        # Remove stopped containers
        for cid in list(self.containers.keys()):
            if cid not in running_ids:
                del self.containers[cid]
        # Add new containers
        for c in running:
            if c["Id"] not in self.containers:
                pid = get_pid(c["Name"])
                if not pid:
                    continue
                paths = get_cgroup_paths(pid, self.version)
                # Get mem limit from inspect
                try:
                    inspect = subprocess.check_output(["sudo", "podman", "inspect", c["Name"]]).decode()
                    info = json.loads(inspect)[0]
                    mem_limit_bytes = info.get("HostConfig", {}).get("Memory", 0)
                except Exception:
                    mem_limit_bytes = 0
                self.containers[c["Id"]] = {
                    "Name": c["Name"],
                    "PID": pid,
                    "Paths": paths,
                    "MemLimit": mem_limit_bytes,
                    "PrevCPU": None,
                    "PrevTime": None,
                    "CPUSum": 0,
                    "Count": 0,
                }

    def print_stats(self):
        print("ID           NAME           CPU %     MEM USAGE / LIMIT   MEM %    PIDS   AVG CPU %")
        for cid, cinfo in self.containers.items():
            mem_usage, mem_limit, mem_percent, cpu_usage, pids_current, pids_max = get_stats_v1(cinfo["Paths"], cinfo["MemLimit"])
            now = time.time()
            prev_cpu = cinfo["PrevCPU"]
            prev_time = cinfo["PrevTime"]
            cpu_sum = cinfo["CPUSum"]
            count = cinfo["Count"]
            if prev_cpu is not None and prev_time is not None:
                delta_cpu = cpu_usage - prev_cpu
                delta_time = now - prev_time
                cpu_pct = (delta_cpu / 1e9) / delta_time * 100 if delta_time > 0 else 0
                cpu_sum += cpu_pct
                count += 1
                avg_cpu_pct = cpu_sum / count
            else:
                cpu_pct = 0
                avg_cpu_pct = 0
            print(format_stats(cid, cinfo["Name"], cpu_pct, mem_usage, mem_limit, mem_percent, pids_current, avg_cpu_pct))
            cinfo["PrevCPU"] = cpu_usage
            cinfo["PrevTime"] = now
            cinfo["CPUSum"] = cpu_sum
            cinfo["Count"] = count

if __name__ == "__main__":
    version = detect_cgroup_version()
    monitor = ContainerMonitor(version)
    try:
        while True:
            monitor.update_containers()
            monitor.print_stats()
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopped.")