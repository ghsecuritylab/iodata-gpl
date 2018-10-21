#!/bin/sh
ifconfig eth0 up
vconfig add eth0 1
ifconfig eth0.1 192.168.99.9 up

