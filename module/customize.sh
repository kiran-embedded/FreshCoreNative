# module/customize.sh
# Executed by the Magisk/KernelSU installer

SKIPUNZIP=0

ui_print " "
ui_print "   ⚙️  FreshCore Native  ⚙️   "
ui_print " "
sleep 0.5

ui_print "- Checking device architecture..."
if [ "$ARCH" != "arm64" ]; then
  abort "! Unsupported architecture: $ARCH. FreshCore requires arm64."
fi

ui_print "- Verifying Android version..."
if [ "$API" -lt 30 ]; then
  ui_print "! Warning: API level $API is older than Android 11."
  ui_print "  FreshCore may run, but some telemetry nodes might be missing."
fi

ui_print "- Extracting native binaries..."
ui_print "  [=====               ] 25%"
sleep 0.2
ui_print "  [==========          ] 50%"
sleep 0.2
ui_print "  [===============     ] 75%"
sleep 0.2
ui_print "  [====================] 100%"

ui_print "- Applying security permissions..."
set_perm_recursive $MODPATH 0 0 0755 0644
set_perm $MODPATH/service.sh 0 0 0755
set_perm $MODPATH/system/bin/freshcore 0 0 0755
set_perm $MODPATH/uninstall.sh 0 0 0755

ui_print " "
ui_print "✔ Installation complete!"
ui_print "  Please reboot to start the daemon."
