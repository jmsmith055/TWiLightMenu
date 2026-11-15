#!/bin/bash
cd "$(dirname "$0")" || exit 1

if ! command -v docker >/dev/null 2>&1; then
    echo "Docker not installed." >&2
    echo "Install Docker at https://www.docker.com/"
    exit 1
fi

if ! docker -v; then
    echo "Docker is not running." >&2
    exit 1
fi

# Rebuild the TWiLightMenu image if it doesn't exist or the Dockerfile has changed since it was built.
dockerfile_hash=$(sha256sum Dockerfile | cut -d' ' -f1)
image_hash=$(docker image inspect twilightmenu --format '{{index .Config.Labels "dockerfile-hash"}}' 2>/dev/null)

if [ "$image_hash" != "$dockerfile_hash" ]; then
    if ! docker build -t twilightmenu --label twilightmenu --label "dockerfile-hash=$dockerfile_hash" ./; then
        echo "Failed to build the Docker image." >&2
        exit 1
    fi
fi

# Only request a TTY when run interactively; "-t -i" fails when input isn't a terminal.
tty_args=()
if [ -t 0 ]; then
    tty_args=(-t -i)
fi

docker run --rm "${tty_args[@]}" -v "$(pwd):/data" twilightmenu make "$@"
