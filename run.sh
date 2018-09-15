#!/bin/bash
qemu-system-x86_64 -L . -m 512 -fda Disk.img -localtime -M pc
