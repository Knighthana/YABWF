#!/usr/bin/env bash

# Brief: Generate clangd configuration for YABWF
# Author: Knighthana (https://github.com/Knighthana)
# Version: docv2.0.0
# Date: 2026-05-23
# any problem please make an issue on https://github.com/Knighthana/YABWF
#
# Generates compile_commands.json via bear (preferred) or falls back
# to a minimal .clangd config file when bear is unavailable or config.h
# has not been generated yet.

PROJECT_PATH=$(dirname "$0")
cd "$PROJECT_PATH" || exit 1
PROJECT_PATH=$(pwd)

CONFIG_H="$PROJECT_PATH/src/config.h"
COMPILE_COMMANDS="$PROJECT_PATH/compile_commands.json"

# --- Attempt 1: bear -- make (preferred) ---
if command -v bear &>/dev/null && [ -f "$CONFIG_H" ]; then
    echo "generator_clangd: using bear -- make to generate compile_commands.json"

    # Clean up old .clangd if present (compile_commands.json takes precedence)
    rm -f "$PROJECT_PATH/.clangd"

    # Run configure if config.status doesn't exist yet
    if [ ! -f "$PROJECT_PATH/config.status" ]; then
        echo "generator_clangd: running ./configure first"
        cd "$PROJECT_PATH" || exit 1
        ./configure || {
            echo "generator_clangd: configure failed, falling back to .clangd"
        }
    fi

    cd "$PROJECT_PATH" || exit 1
    bear -- make -C src clean 2>/dev/null
    bear -- make -C src -j"$(nproc)" 2>/dev/null

    if [ -f "$COMPILE_COMMANDS" ]; then
        echo "generator_clangd: compile_commands.json generated successfully"
        exit 0
    else
        echo "generator_clangd: bear produced no output, falling back to .clangd"
    fi
else
    if ! command -v bear &>/dev/null; then
        echo "generator_clangd: bear not found, generating fallback .clangd"
    fi
    if [ ! -f "$CONFIG_H" ]; then
        echo "generator_clangd: config.h not found (project not configured), generating fallback .clangd"
    fi
fi

# --- Fallback: generate minimal .clangd ---
rm -f "$COMPILE_COMMANDS"

cat > "$PROJECT_PATH/.clangd" << EOF
CompileFlags:
  Compiler: gcc
  Add: [
    -I$PROJECT_PATH/src,
    -I.,
    -DHAVE_CONFIG_H,
    -DSUPPORT_ACCESS_CONTROL,
    -DUSE_POLL,
    -Os
  ]
EOF

echo "generator_clangd: .clangd fallback config generated (include path: $PROJECT_PATH/src)"
echo "generator_clangd: run 'bear -- make' after './configure' for full compilation database"
