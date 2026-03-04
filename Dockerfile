# Codename: Subspace - C++ Engine Dockerfile
# Multi-stage build for minimal runtime image

# --- Build stage ---
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy engine source
COPY engine/ engine/
COPY GameData/ GameData/

# Build engine
RUN mkdir -p engine/build \
    && cd engine/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DSUBSPACE_BUILD_TESTS=OFF \
    && cmake --build . --config Release -j$(nproc)

# --- Runtime stage ---
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libgl1-mesa-glx \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash subspace

WORKDIR /opt/subspace

# Copy built binary and game data
COPY --from=builder /build/engine/build/subspace_game .
COPY --from=builder /build/GameData/ GameData/

RUN chown -R subspace:subspace /opt/subspace

USER subspace

ENTRYPOINT ["./subspace_game"]
