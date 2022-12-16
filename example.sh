# !/bin/bash
# Default serial port maybe on /dev/ttyUSB0
port=${1:-"/dev/ttyUSB0"}
./build/serial -p ${port} -b 9600

if [ $? -ne 0 ]; then
    exit 1
fi
echo "OK"
exit 0