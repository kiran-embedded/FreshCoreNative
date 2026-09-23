#!/system/bin/sh
MODDIR=${0%/*}

# Wait for boot to finish completely
until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 1
done

# Sleep a bit more to let the system settle
sleep 30

# Start freshcore daemon in the background
nohup ${MODDIR}/system/bin/freshcore > /dev/null 2>&1 &
