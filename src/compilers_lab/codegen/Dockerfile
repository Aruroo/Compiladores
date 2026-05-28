FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    flex \
    bison \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Copy source files
WORKDIR /app
COPY src/ .

# Build
RUN make

# Run
ENTRYPOINT ["./carta"]