# Container Monitor for Automotive & Embedded Systems

## Overview

This project is a highly configurable, real-time container resource monitoring solution specifically designed for automotive and embedded environments. It enables precise control over how container metrics are collected, visualized, and exported, making it ideal for use cases where timing, reliability, and low system overhead are critical.

---

## Why Is This Project Important?

- **Automotive & Embedded Focus:**  
  Modern vehicles and embedded systems increasingly rely on containerized applications for modularity and security. Monitoring these containers in real time is essential for diagnostics, validation, and safety.
- **Fine-Grained Control:**  
  Unlike generic monitoring tools, this project allows you to tune sampling rates, batch sizes, thread counts, and alert thresholds to match the requirements of your hardware and use case.
- **Operator & Developer Friendly:**  
  The project provides a real-time dashboard, safe configuration via a GUI, and export options for historical analysis, making it suitable for both engineers and operators in test benches, HIL/SIL setups, and in-vehicle systems.
- **Reliability & Safety:**  
  Centralized configuration and validation help prevent human error, supporting safe deployment in critical environments.

---

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
   - Batch processing and tunable thread pools allow efficient metric collection, even on resource-constrained hardware.
   - Sampling rates and refresh intervals can be adjusted to balance performance and overhead.

5. **Historical Tracking & Export:**
   - Metrics can be exported to CSV files or a database for compliance, traceability, and further analysis.

6. **Alerting:**
   - Configurable warning and critical thresholds trigger color-coded alerts in the UI, helping operators quickly identify issues.

7. **Documentation & Extensibility:**
   - The codebase is modular and documented with Doxygen, making it easy to extend for new metrics, storage backends, or integration with automotive-grade systems.

---

## Typical Use Cases

- **Automotive Test Benches:**  
  Monitor containerized applications running on ECUs or gateways during validation and testing.
- **Embedded Systems:**  
  Track resource usage and health of containers in real time, with minimal overhead.
- **Operator Consoles:**  
  Provide a live dashboard for engineers and operators to monitor system status and respond to alerts.
- **Compliance & Traceability:**  
  Export historical metrics for analysis, reporting, and regulatory compliance.

---

## Getting Started

1. **Configure Parameters:**  
   Use the GUI to generate a valid `parameter.conf` file.
2. **Build & Run:**  
   Build the project using CMake and run the main application.
3. **Monitor & Export:**  
   View live metrics in the dashboard and export data as needed.

---

## Next Steps

In the following sections, we will compare this project with other existing container monitoring solutions and highlight its unique advantages for automotive and embedded use cases.

# Container Monitor for Automotive & Embedded Systems

A highly configurable, real-time container resource monitoring solution designed for automotive and embedded use cases. This project provides fine-grained control over sampling rates, resource management, and UI visualization, making it ideal for environments where timing, reliability, and low overhead are critical.

---

## Key Features

- **Automotive-Focused Design:**  
  - Lightweight, modular C++ codebase suitable for ECUs, gateways, and test benches.
  - Direct cgroup kernel file access for accurate, low-latency resource metrics.
  - Explicit control over sampling intervals, batch size, thread count, and alert thresholds.

- **Configurable & Safe:**  
  - All runtime parameters are managed via a single `parameter.conf` file.
  - Includes a Tkinter-based GUI for config generation and validation, preventing human error.

- **Real-Time Dashboard:**  
  - Ncurses-based UI with color-coded metrics and dynamic column alignment.
  - Displays only live containers, removes stale ones, and supports operator visibility.

- **Resource Management:**  
  - Thread pool and batch processing for efficient metric collection.
  - Tunable for real-time or near-real-time operation.

- **Historical Tracking & Export:**  
  - Supports CSV and database export for compliance, traceability, and analysis.

- **Alerting & Safety Integration:**  
  - Configurable warning and critical thresholds.
  - Color-coded UI for quick status assessment.

- **Maintainable & Extensible:**  
  - Modular architecture with Doxygen documentation.
  - Easy to extend for new metrics, storage backends, or event sources.

---

## Why This Project Stands Out

- **Compared to General Tools:**  
  Unlike `cmonitor`, `cAdvisor`, `podman stats`, or `docker stats`, this project is built for automotive and embedded constraints, offering ms-level control, low overhead, and robust configuration.
- **Validation & Safety:**  
  The included config generator ensures only valid parameters are used, supporting safe deployment in critical environments.
- **UI & Operator Experience:**  
  Advanced dashboard features and alerting make it suitable for both developers and operators.

---

## Getting Started

1. **Configure Parameters:**
   - Use the provided Tkinter GUI (`create_config_gui.py`) to generate a valid `parameter.conf`.
   - All limits and options are enforced to prevent misconfiguration.

2. **Build & Run:**
   - Build the project using CMake.
   - Run the main application; the dashboard will display live container metrics.

3. **Export & Analyze:**
   - Metrics can be exported to CSV or database for further analysis.

---

## Requirements

- Linux system with cgroup v1 or v2 support.
- C++17 compiler, CMake.
- Python 3.x (for config GUI).
- Ncurses library.
- SQLite (for database export, if enabled).

---

## Extending

- Add support for additional metrics or container runtimes.
- Integrate with automotive-grade databases or event systems.
- Enhance UI for custom operator workflows.

---

## Alternatives

- [cmonitor](https://github.com/f18m/cmonitor/tree/master): General-purpose, less configurable, no advanced UI.
- [cAdvisor](https://github.com/google/cadvisor): Powerful but heavy, not optimized for embedded/automotive.
- [podman stats](https://docs.podman.io/en/latest/markdown/podman-stats.1.html): Live stats, limited configurability.
- [docker stats](https://docs.docker.com/engine/reference/commandline/stats/): Similar to podman stats.
- [Sysdig](https://sysdig.com/opensource/): Advanced system exploration, higher overhead.

---

## Documentation

- All major source files are documented with Doxygen.
- See `docs/doxygen/html/index.html` after running Doxygen for full API documentation.

---

## License

This project is licensed under the MIT License.

---

**Designed for reliability, configurability, and maintainability in automotive and embedded container environments.**