docker run -it --rm \
  --add-host=host.docker.internal:host-gateway \
  -v "$(pwd)":/workspace \
  -w /workspace \
  ghcr.io/anomalyco/opencode