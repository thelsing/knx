#!/usr/bin/env python
from subprocess import run
from datetime import datetime, timedelta
from os.path import expanduser

ocddir = expanduser("~/.platformio/packages/tool-openocd/")
chip = "stm32f1x"

def unlock(*args, **kwargs):
  print("Please connect the board within the next two minutes.")
  endtime = datetime.now() + timedelta(minutes = 1)
  ret = 1
  while ret != 0 and datetime.now() < endtime:
    ret = run(["bin/openocd", "-f", "interface/stlink.cfg", "-f", "target/" + chip + ".cfg", "-c", "init", "-c", "reset halt", "-c", chip + " unlock 0", "-c", "reset halt", "-c", "exit"], cwd = ocddir).returncode
  if ret != 0:
    print("Timeout")
  return ret

try:
  Import("env")
except NameError:
  import sys
  if len(sys.argv) > 1:
    chip = sys.argv[1]
  if len(sys.argv) > 2:
    ocddir = sys.argv[2]
  unlock(None, None)
else:
  ocddir = env.PioPlatform().get_package_dir("tool-openocd")
  options = env.GetProjectOptions()
  for option in options:
    if "unlock_chip" == option[0]:
      chip = option[1]
  env.AddCustomTarget("unlock", None, unlock)
