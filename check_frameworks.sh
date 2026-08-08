#!/usr/bin/env bash
APP="build/Frontend/EmailClusteringFrontend.app"

for f in "$APP"/Contents/Frameworks/*/Versions/A/* "$APP"/Contents/PlugIns/*/*; do
    if [ -f "$f" ] && strings "$f" 2>/dev/null | grep -q "opt/homebrew"; then
        echo "=== $f ==="
        strings "$f" | grep "opt/homebrew"
    fi
done
