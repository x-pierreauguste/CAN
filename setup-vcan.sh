#!/bin/bash

# load kernel module for VCAN support
sudo modprobe vcan

# add virtual interface
ip link add dev vcan0 type vcan

# configure and bring up the interface
ip link set up vcan0

echo "VCAN ONLINE"
echo "OPTIONS:"
echo "      candump vcan0"
echo "      cangen vcan0"