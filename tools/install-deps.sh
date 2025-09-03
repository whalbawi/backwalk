#!/usr/bin/env bash
set -eux

sudo apt-get update

sudo apt-get install -y \
    clang-19 \
    clang-format-19 \
    clang-tidy-19 \
    iwyu

sudo ln -sf /usr/bin/clang-19 /usr/bin/clang
sudo ln -sf /usr/bin/clang-format-19 /usr/bin/clang-format
sudo ln -sf /usr/bin/clang-tidy-19 /usr/bin/clang-tidy