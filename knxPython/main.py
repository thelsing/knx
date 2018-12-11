import knx
import time
import sys

print("start")
while True:
    time.sleep(1)
    cmd = sys.stdin.read(1)
    if cmd == 'q':
        break
    elif cmd == 's':
        print("start knx")
        name = knx.Start()
    elif cmd == 'p':
        currentMode = knx.ProgramMode(not knx.ProgramMode())
        print("set programming mode to " + str(currentMode))
print("end")