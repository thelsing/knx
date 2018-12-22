#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
    
namespace py = pybind11;

#include <Python.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <thread>
#include <stdint.h>
#include <vector>

#include "linux_platform.h"
#include "knx/bau57B0.h"
#include "knx/group_object_table_object.h"


LinuxPlatform platform;
Bau57B0 bau(platform);

#if 1 // this code will go to python later
//float currentValue = 0;
//float maxValue = 0;
//float minValue = RAND_MAX;
//long lastsend = 0;
//
//GroupObject groupObjects[]
//{
//    GroupObject(2),
//    GroupObject(2),
//    GroupObject(2),
//    GroupObject(1)
//}
//;
//#define CURR groupObjects[0]
//#define MAX groupObjects[1]
//#define MIN groupObjects[2]
//#define RESET groupObjects[3]
//
//void measureTemp()
//{
//    long now = platform.millis();
//    if ((now - lastsend) < 2000)
//        return;
//
//    lastsend = now;
//    int r = rand();
//    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
//    currentValue *= 100 * 100;
//    
//    CURR.objectWrite(currentValue);
//
//    if (currentValue > maxValue)
//    {
//        maxValue = currentValue;
//        MAX.objectWrite(maxValue);
//    }
//
//    if (currentValue < minValue)
//    {
//        minValue = currentValue;
//        MIN.objectWrite(minValue);
//    }
//}

//void resetCallback(GroupObject& go)
//{
//    if (go.objectReadBool())
//    {
//        maxValue = 0;
//        minValue = 10000;
//    }
//}

//void appLoop()
//{
//    if (!bau.configured())
//        return;
//    
//    measureTemp();
//}

void setup()
{
    srand((unsigned int)time(NULL));
    bau.readMemory();
    
//    GroupObjectTableObject& got(bau.groupObjectTable());
//    got.groupObjects(groupObjects, 4);
    
    DeviceObject& devObj(bau.deviceObject());
    devObj.manufacturerId(0xfa);

//    RESET.updateHandler = resetCallback;

    bau.enabled(true);
}
#endif

bool threadEnabled = false;
static void loop()
{
    while (1)
    {
        bau.loop();
        //appLoop();
        platform.mdelay(100);
    }
}

static std::thread workerThread;

bool started = false;
static void Start()
{
    if (started)
        return;
    
    setup();

    workerThread = std::thread(loop);
    workerThread.detach();
}

static bool ProgramMode(bool value)
{
    bau.deviceObject().progMode(value);
    return bau.deviceObject().progMode();
}

static bool ProgramMode()
{
    return bau.deviceObject().progMode();
}

static void RegisterGroupObjects(std::vector<GroupObject>& gos)
{
    GroupObjectTableObject& got(bau.groupObjectTable());
    got.groupObjects(gos.data(), gos.size());
}

PYBIND11_MAKE_OPAQUE(std::vector<GroupObject>);

PYBIND11_MODULE(knx, m) {
    m.doc() = "wrapper for knx device lib";   // optional module docstring

    py::bind_vector<std::vector<GroupObject>>(m, "GroupObjectList");
    
    m.def("Start", &Start, "Start knx handling thread.");
    m.def("ProgramMode", (bool (*)())&ProgramMode, "get programing mode active.");
    m.def("ProgramMode", (bool(*)(bool))&ProgramMode, "Activate / deactivate programing mode.");
    m.def("RegisterGroupObjects", &RegisterGroupObjects);
    
    py::class_<GroupObject>(m, "GroupObject")
        .def(py::init<uint8_t>())
        .def("objectWrite", (void(GroupObject::*)(float))&GroupObject::objectWrite);
}