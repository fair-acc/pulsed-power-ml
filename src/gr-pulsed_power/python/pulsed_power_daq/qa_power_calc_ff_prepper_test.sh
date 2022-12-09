#!/usr/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir="/home/johanna/gr-digitizers/python"
export GR_CONF_CONTROLPORT_ON=False
export PATH="/home/johanna/gr-digitizers/python":"$PATH"
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/home/johanna/gr-digitizers/test_modules:$PYTHONPATH
/usr/bin/python3 /home/johanna/gr-digitizers/python/qa_power_calc_ff_prepper.py 
