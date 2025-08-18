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

# CMD show help if no command is given
CMD ["stress-ng", "--help"]