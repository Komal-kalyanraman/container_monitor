# Container Monitor for Automotive Systems

## Overview

This project is a highly configurable, real-time container resource monitoring solution specifically designed for the emerging domain of **Software Defined Vehicles (SDV)** and embedded environments. As the automotive industry transitions toward SDV architectures, containers are becoming an integral part of in-vehicle software deployment. This tool enables precise control over how container metrics are collected, visualized, and exported, making it ideal for diagnostics, validation, and post-analysis in SDV and embedded use cases where timing, reliability, and low system overhead are critical.

## Features

- **Automotive & SDV Focus:**  
  - Designed for in-vehicle, embedded, and test-bench environments.
  - Real-time monitoring of containerized workloads on ECUs and gateways.
- **Fine-Grained Control:**  
  - Tune sampling rates, batch sizes, thread counts, and alert thresholds via config.
- **Diagnostics & Post-Analysis:**  
  - Live dashboard for operators and engineers.
  - Export metrics to CSV or database for traceability and compliance.
- **Operator & Developer Friendly:**  
  - Ncurses-based UI with color-coded alerts and dynamic alignment.
  - Tkinter GUI for safe, validated configuration file generation.
- **Resource Management:**  
  - Thread pool and batch processing for efficient metric collection.
- **Extensible & Maintainable:**  
  - Modular C++ codebase, Doxygen documentation, and easy integration with new metrics or storage backends.
- **Centralized Configuration:**  
  - All runtime parameters in a single `parameter.conf` file.

## Folder Structure

```
App/
├── analysis/           # Analysis logic for live metrics aggregation
├── build/              # Build artifacts (CMake, binaries, etc.)
├── container_runtime/  # Container runtime path factories and configuration
├── database/           # Database interface and SQLite implementation
├── metrics_analyzer/   # Metrics reading and analysis logic
├── monitoring_service/ # Event listeners, processors, resource monitoring, thread pool
├── thirdparty/         # External dependencies (if any)
├── ui/                 # Ncurses dashboard and UI logic
├── utils/              # Common utilities (config parsing, logging, common types)
└── main.cpp            # Application entry point

config/
├── create_config_gui.py   # Tkinter GUI for safe config generation
└── parameter.conf         # Centralized configuration file

storage/
├── container_metrics.csv  # Exported container metrics
├── host_usage.csv         # Exported host metrics
└── metrics.db             # SQLite database (if enabled)

post_analysis/
└── plot_container_metrics.py # Scripts for plot creation for further analysis
```

## Quick Start

1. **Install Requirements**
   - Linux with cgroup v1 or v2 support
   - C++17 compiler, CMake
   - Python 3.x (for config GUI)
   - Ncurses library
   - SQLite (for database export, if enabled)

2. **Generate Configuration**
   - Run the GUI:  
     ```bash
     python3 config/create_config_gui.py
     ```
   - Fill in parameters and save to generate `parameter.conf`.

3. **Build the Project**
   - From the root directory:
     ```bash
     mkdir -p App/build
     cd App/build
     cmake ..
     make
     ```

4. **Run the Monitor**
   - From the build directory:
     ```bash
     ./container_monitor
     ```
   - The ncurses dashboard will display live container metrics.

5. **Export & Analyze**
   - Metrics are exported to CSV/database in the `storage/` folder for post-analysis.

## How Is This Solution Better Than Other Tools?

- [**cAdvisor**](https://github.com/google/cadvisor):  
  Heavyweight, designed for cloud/Kubernetes, not for embedded/SDV. No real-time dashboard, limited sampling control, and not easily configurable for automotive use cases.

- [**cmonitor**](https://github.com/f18m/cmonitor/tree/master):  
  General-purpose, console-based, Python (higher overhead), limited configurability, no advanced UI, and not tailored for SDV/automotive.

- **Grafana/Prometheus:**  
  Powerful for cloud/enterprise, but requires complex setup, not real-time, and not suitable for embedded/SDV without significant adaptation.

- [**podman stats**](https://docs.podman.io/en/latest/markdown/podman-stats.1.html) / [**docker stats**](https://docs.docker.com/engine/reference/commandline/stats/):  
  Simple CLI tools, no historical export, no alerting, no dashboard, and no configurability for sampling or resource management.

## Documentation

- All major source files are documented with Doxygen.
- See `docs/doxygen/html/index.html` after running Doxygen for full API documentation.

## License

This project is licensed under the MIT License.