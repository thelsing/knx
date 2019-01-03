import knx
import time
import sys

def updated(g1):
    print(g1.value)

print("start")
gos = knx.GroupObjectList()
gos.append(knx.GroupObject(2))
gos.append(knx.GroupObject(2))
gos.append(knx.GroupObject(2))
gos.append(knx.GroupObject(1))
curr = gos[0]
min = gos[1]
max = gos[2]
reset = gos[3]
reset.callBack(updated)

knx.RegisterGroupObjects(gos)
knx.Start()
while True:
    time.sleep(1)
    cmd = sys.stdin.read(1)
    if cmd == 'q':
        break
    elif cmd == 'p':
        currentMode = knx.ProgramMode(not knx.ProgramMode())
        print("set programming mode to " + str(currentMode))
    elif cmd == 'w':
        cmd = sys.stdin.read(4)
        value = float(cmd)
        curr.objectWrite(value)
        print("wrote " + str(value) + " to curr")
print("end")