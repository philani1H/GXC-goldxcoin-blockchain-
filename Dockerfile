FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    cmake \
    build-essential \
    netcat \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy application files
COPY . /app

# Install Python dependencies
RUN pip3 install --no-cache-dir -r requirements.txt

# Make scripts executable
RUN chmod +x start_complete_ecosystem.sh stop_complete_ecosystem.sh build_miners_installers.sh

# Create necessary directories
RUN mkdir -p gxc_data gxc_testnet_data logs pids miners/installers

# Expose ports
EXPOSE 18332 8080 8081 3000 3001 3333 3334 3335 5000 6000 6001 6002

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
    CMD curl -f http://localhost:3000 || exit 1

# Start services
CMD ["bash", "-c", "./start_complete_ecosystem.sh && tail -f logs/*.log"]
