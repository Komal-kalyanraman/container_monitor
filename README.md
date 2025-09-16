# Container Monitor for Software Defined Vehicles (SDV) & Embedded Systems

## Overview

This project is a highly configurable, real-time container resource monitoring solution specifically designed for the emerging domain of **Software Defined Vehicles (SDV)** and embedded environments. As the automotive industry transitions toward SDV architectures, containers are becoming an integral part of in-vehicle software deployment. This tool enables precise control over how container metrics are collected, visualized, and exported, making it ideal for diagnostics, validation, and post-analysis in SDV and embedded use cases where timing, reliability, and low system overhead are critical.

## Why Is This Project Important?

- **SDV & Automotive Focus:**  
  Modern vehicles are evolving into software platforms, with containers enabling modular, updatable, and secure software stacks. Real-time monitoring of these containers is essential for diagnostics, validation, and ensuring system health in SDV architectures.
- **Fine-Grained Control:**  
  Unlike generic monitoring tools, this project allows you to tune sampling rates, batch sizes, thread counts, and alert thresholds to match the requirements of your SDV hardware and use case.
- **Diagnostics & Post-Analysis:**  
  The tool is designed to support both live diagnostics (for engineers and operators) and post-analysis (via export to CSV/database), making it valuable throughout the SDV software lifecycle.
- **Operator & Developer Friendly:**  
  Provides a real-time dashboard, safe configuration via a GUI, and export options for historical analysis, making it suitable for both engineers and operators in test benches, HIL/SIL setups, and in-vehicle systems.
- **Reliability & Safety:**  
  Centralized configuration and validation help prevent human error, supporting safe deployment in critical SDV environments.

## How Does It Work?

1. **Configuration:**
   - All runtime parameters (sampling intervals, batch size, thread count, alert thresholds, etc.) are set in a single `parameter.conf` file.
   - A Tkinter-based GUI (`create_config_gui.py`) is provided to generate and validate this config file, ensuring only valid values are used.

2. **Metric Collection:**
   - The system uses a thread pool to collect CPU, memory, and PID metrics from all running containers.
   - Metrics are read directly from kernel cgroup files for accuracy and low latency.

3. **Real-Time Dashboard:**
   - An ncurses-based UI displays live metrics for each container.
   - The dashboard features color-coded alerts, dynamic column alignment, and only shows active containers.

4. **Resource Management:**
   - Batch processing and tunable thread pools allow efficient metric collection, even on resource-constrained SDV hardware.
   - Sampling rates and refresh intervals can be adjusted to balance performance and overhead.

5. **Historical Tracking & Export:**
   - Metrics can be exported to CSV files or a database for compliance, traceability, and further analysis—supporting post-analysis and diagnostics.

6. **Alerting:**
   - Configurable warning and critical thresholds trigger color-coded alerts in the UI, helping operators quickly identify issues.

7. **Documentation & Extensibility:**
   - The codebase is modular and documented with Doxygen, making it easy to extend for new metrics, storage backends, or integration with automotive-grade systems.

## Typical Use Cases

- **SDV Diagnostics & Validation:**  
  Monitor containerized applications running on ECUs or gateways during validation, testing, and in-field diagnostics in SDV platforms.
- **Embedded Systems:**  
  Track resource usage and health of containers in real time, with minimal overhead.
- **Operator Consoles:**  
  Provide a live dashboard for engineers and operators to monitor system status and respond to alerts.
- **Compliance & Traceability:**  
  Export historical metrics for analysis, reporting, and regulatory compliance in automotive environments.

## How Is This Solution Better Than Other Tools?

- [**cAdvisor**](https://github.com/google/cadvisor):  
  Heavyweight, designed for cloud/Kubernetes, not for embedded/SDV. No real-time dashboard, limited sampling control, and not easily configurable for automotive use cases.

- [**cmonitor**](https://github.com/f18m/cmonitor/tree/master):  
  General-purpose, console-based, Python (higher overhead), limited configurability, no advanced UI, and not tailored for SDV/automotive.

- **Grafana/Prometheus:**  
  Powerful for cloud/enterprise, but requires complex setup, not real-time, and not suitable for embedded/SDV without significant adaptation.

- [**podman stats**](https://docs.podman.io/en/latest/markdown/podman-stats.1.html) / [**docker stats**](https://docs.docker.com/engine/reference/commandline/stats/):  
  Simple CLI tools, no historical export, no alerting, no dashboard, and no configurability for

## Getting Started

1. **Configure Parameters:**
   - Use the provided Tkinter GUI (`create_config_gui.py`) to generate a valid `parameter.conf`.
   - All limits and options are enforced to prevent misconfiguration.

2. **Build & Run:**
   - Build the project using CMake.
   - Run the main application; the dashboard will display live container metrics.

3. **Export & Analyze:**
   - Metrics can be exported to CSV or database for further analysis.

## Requirements

- Linux system with cgroup v1 or v2 support.
- C++17 compiler, CMake.
- Python 3.x (for config GUI).
- Ncurses library.
- SQLite (for database export, if enabled).

## Documentation

- All major source files are documented with Doxygen.
- See `docs/doxygen/html/index.html` after running Doxygen for full API documentation.

## License

This project is licensed under the MIT License.