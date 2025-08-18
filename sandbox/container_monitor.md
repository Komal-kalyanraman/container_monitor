# Automotive Container Monitor

## **Project Overview**

Build a standalone, real-time container resource monitor for automotive environments that tracks CPU, Memory, and PID usage against allocated limits with red/green alerting. Runtime is specified at startup, eliminating auto-discovery overhead.

***

## **Phase 1: Core System Foundation**

### **Step 1.1: Container Enumeration \& Limit Extraction**

**Objective:** Query containers from specified runtime and extract resource limits

**Implementation:**

- Runtime selection via command-line flag: `--runtime=docker` or `--runtime=podman`
- Query all running containers via runtime APIs
- Extract resource limits directly from `docker inspect` / `podman inspect`
- Parse container PIDs and cgroup paths

**Key Learning:**

- Container runtime internals
- JSON parsing and system command execution
- Runtime-specific API differences

**Deliverables:**

- `ContainerEnumerator` class
- Runtime-specific limit extraction
- Container metadata structures

***

### **Step 1.2: Low-Level Resource Collection**

**Objective:** Implement direct /proc and cgroups parsing for maximum performance

**Implementation:**

- Direct /proc filesystem parsing (no high-level libraries)
- Manual cgroup v1/v2 file reading for container processes
- Raw syscalls for file operations (open, read, close)
- Efficient data structures for metrics storage

**Key Learning:**

- /proc/[pid]/stat, /proc/[pid]/statm parsing
- cgroups subsystem internals
- Direct syscall usage
- Linux kernel data structures

**Deliverables:**

- `LowLevelResourceCollector` class
- /proc parser implementations
- cgroups reader for container limits verification

***

### **Step 1.3: Real-Time Data Collection Loop**

**Objective:** High-frequency, low-latency metric collection with minimal overhead

**Implementation:**

- Real-time scheduling (SCHED_FIFO)
- CPU affinity pinning to dedicated core
- Memory locking (mlockall) to prevent swapping
- High-resolution timing (clock_gettime with CLOCK_MONOTONIC_RAW)
- Lock-free data structures for metrics queues

**Key Learning:**

- Real-time Linux programming
- Scheduling policies and priorities
- Memory management and page locking
- High-precision timing mechanisms

**Deliverables:**

- `RealtimeCollector` class
- Performance-optimized collection loop
- Sub-millisecond timing accuracy

***

## **Phase 2: Resource Limit Analysis**

### **Step 2.1: Resource Limit Processing**

**Objective:** Parse and normalize container resource limits from runtime inspect data

**Implementation:**

- Parse Docker/Podman inspect JSON for HostConfig limits
- Handle different limit formats (shares, quotas, bytes)
- Convert to standardized units (CPU cores, MB, PID count)
- Cross-reference with cgroups limits as verification

**Key Learning:**

- Container resource allocation mechanisms
- Units conversion and normalization
- JSON parsing performance optimization

**Deliverables:**

- `ResourceLimitProcessor` class
- Standardized limit data structures
- Units conversion utilities

***

### **Step 2.2: Usage vs Limit Analysis Engine**

**Objective:** Calculate utilization percentages and generate automotive-grade alerts

**Implementation:**

- Real-time usage percentage calculations (actual/limit * 100)
- Automotive threshold-based alerting:
    - **GREEN**: < 80% (SAFE)
    - **YELLOW**: 80-95% (WARNING)
    - **ORANGE**: 95-100% (CRITICAL)
    - **RED**: > 100% (VIOLATION)
- Moving average calculations for stability
- Violation detection and duration tracking

**Key Learning:**

- Mathematical calculations for resource utilization
- Automotive-specific alerting thresholds
- Statistical analysis for noise reduction

**Deliverables:**

- `UtilizationAnalyzer` class
- Alert level determination logic
- Violation tracking and logging

***

## **Phase 3: Performance Monitoring \& Optimization**

### **Step 3.1: Built-in Performance Profiling**

**Objective:** Self-monitoring and instrumentation for debug tool integration

**Implementation:**

- perf event counter integration for CPU cycles/cache misses
- Custom memory allocation tracking with debug hooks
- Timing instrumentation for hot code paths
- CPU usage self-monitoring to ensure <2% overhead

**Key Learning:**

- perf event API programming
- Performance counter analysis
- Memory allocation optimization
- Self-monitoring techniques

**Deliverables:**

- `PerformanceProfiler` class
- Built-in benchmarking capabilities
- Debug tool integration points

***

### **Step 3.2: Debug Tool Integration Architecture**

**Objective:** Force systematic usage of Linux debugging tools

**Implementation:**

- Custom memory allocators for valgrind leak detection
- GDB-friendly debug symbols and signal handlers
- Built-in breakpoint locations for common debugging scenarios
- Structured logging for strace/ltrace analysis

**Key Learning:**

- valgrind, gdb, strace, perf, gprof mastery
- Memory debugging techniques
- Performance profiling workflows
- Production debugging strategies

**Deliverables:**

- Debug-instrumented code architecture
- Systematic tool usage workflows
- Production debugging capabilities

***

## **Phase 4: Real-Time Terminal Interface**

### **Step 4.1: Efficient ncurses-Based Dashboard**

**Objective:** Create automotive-grade terminal interface with minimal resource usage

**Implementation:**

- ncurses-based real-time dashboard
- Differential screen updates (only changed regions)
- Color-coded automotive alerts (GREEN/YELLOW/ORANGE/RED)
- Keyboard navigation for container selection and filtering

**Key Learning:**

- ncurses programming and optimization
- Efficient terminal rendering techniques
- Real-time UI update strategies
- Memory-efficient screen management

**Deliverables:**

- `AutomotiveTerminalUI` class
- Real-time container dashboard
- Interactive monitoring interface

***

### **Step 4.2: Automotive Alert Visualization**

**Objective:** Present resource violations with clear automotive-grade indicators

**Implementation:**

- Real-time utilization progress bars
- Container status indicators with automotive color coding
- System resource summary panel
- Violation history and duration display

**Key Learning:**

- Data visualization for safety-critical systems
- Automotive HMI design principles
- Information hierarchy for debugging

**Deliverables:**

- Visual automotive alert system
- Container resource dashboard
- Safety-critical information display

***

## **Phase 5: Data Persistence \& Export**

### **Step 5.1: Lightweight Time-Series Storage**

**Objective:** Store historical data with minimal memory/CPU overhead

**Implementation:**

- SQLite embedded database with WAL mode
- Optimized time-series schema for automotive compliance
- Automatic data retention with configurable periods
- Transaction batching for performance

**Key Learning:**

- Database design for automotive time-series data
- SQLite optimization for embedded systems
- Data retention strategies for compliance

**Deliverables:**

- `AutomotiveTimeSeriesDB` class
- Historical data persistence
- Automotive compliance data storage

***

### **Step 5.2: Automotive Data Export Engine**

**Objective:** Export data for compliance reporting and analysis

**Implementation:**

- CSV export for spreadsheet analysis
- JSON export for automation/integration
- ISO 26262 compliance report generation
- Configurable time range and filtering

**Key Learning:**

- Automotive data export formats
- Compliance reporting requirements
- Efficient data serialization

**Deliverables:**

- `AutomotiveDataExporter` class
- ISO 26262 compliance reports
- Integration-ready data formats

***

## **Phase 6: Python Analysis Tools**

### **Step 6.1: Automotive Analysis Dashboard**

**Objective:** Python-based post-analysis tools for automotive validation

**Implementation:**

- Interactive Plotly/Dash automotive compliance dashboard
- Historical trend analysis with violation tracking
- Statistical analysis for resource pattern detection
- ISO 26262 compliance report generation

**Key Learning:**

- Python data analysis for automotive systems
- Automotive compliance visualization
- Statistical analysis techniques

**Deliverables:**

- `automotive_analyzer.py` - Python analysis toolkit
- Interactive compliance dashboard
- Automotive validation reports

***

## **Simplified Startup \& Usage**

### **Command Line Interface:**

```bash
# Docker runtime
./automotive_container_monitor --runtime=docker

# Podman runtime  
./automotive_container_monitor --runtime=podman

# With optional configuration
./automotive_container_monitor --runtime=docker --update-interval=100ms --cpu-affinity=0
```


### **Configuration File Support:**

```yaml
# automotive_monitor.conf
runtime: docker
update_interval_ms: 100
cpu_affinity_core: 0
alert_thresholds:
  warning: 80.0
  critical: 95.0
  violation: 100.0
storage:
  database_path: "/var/automotive_monitor/metrics.db"
  retention_days: 30
```


***

## **Technical Requirements**

### **Performance Targets:**

- **Collection Latency:** <1ms per container
- **Memory Footprint:** <20MB total application memory
- **CPU Overhead:** <2% system CPU usage
- **Update Frequency:** 100Hz (10ms intervals) capability
- **Container Capacity:** Support 100+ containers simultaneously


### **Automotive-Specific Requirements:**

- **Deterministic startup time:** <500ms from launch to monitoring
- **Resource violation detection:** <10ms response time
- **Compliance logging:** All violations logged with nanosecond timestamps
- **Graceful degradation:** Continue monitoring if individual containers fail

***

## **Final Architecture (Simplified)**

```
AutomotiveContainerMonitor/
├── src/
│   ├── enumeration/
│   │   └── container_enumerator.cpp     # Runtime-specific container listing
│   ├── collection/
│   │   ├── proc_parser.cpp             # Direct /proc parsing  
│   │   ├── cgroup_reader.cpp           # cgroups verification
│   │   └── realtime_collector.cpp      # High-frequency collection
│   ├── analysis/
│   │   ├── limit_processor.cpp         # Runtime limit extraction
│   │   ├── utilization_analyzer.cpp    # Usage vs limit analysis
│   │   └── automotive_alerts.cpp       # Automotive alert generation
│   ├── ui/
│   │   ├── automotive_terminal.cpp     # ncurses automotive interface
│   │   └── alert_renderer.cpp          # Visual automotive alerts
│   ├── storage/
│   │   ├── automotive_timeseries.cpp   # SQLite automotive data
│   │   └── compliance_exporter.cpp     # ISO 26262 export
│   └── profiling/
│       ├── perf_monitor.cpp            # Self-performance monitoring
│       └── debug_hooks.cpp             # Debug tool integration
├── tools/
│   ├── automotive_analyzer.py         # Python compliance analysis
│   └── compliance_reporter.py         # ISO 26262 reporting
└── automotive_monitor_config.yaml     # Simple configuration
```
