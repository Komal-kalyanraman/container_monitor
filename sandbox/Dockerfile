FROM ubuntu:24.04

LABEL description="Versatile stress-testing image for container resource monitoring"

# Install stress, stress-ng, and common tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        stress \
        stress-ng \
        iperf3 \
        fio \
        procps \
        net-tools \
        python3 \
        && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set default shell
SHELL ["/bin/bash", "-c"]

# Run stress-ng in the background to keep CPU, memory, and pids fluctuating
CMD ["stress-ng", "--cpu", "2", "--vm", "2", "--vm-bytes", "256M", "--fork", "4", "--timeout", "0"]