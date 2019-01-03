#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
    
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

static void loop()
{
    while (1)
    {
        bau.loop();
        platform.mdelay(100);
    }
}

static std::thread workerThread;

bool started = false;
static void Start()
{
    if (started)
        return;
    
    started = true;
    
    bau.readMemory();
    bau.enabled(true);

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

PYBIND11_MODULE(knx, m) 
{
    m.doc() = "wrapper for knx device lib";    // optional module docstring

    py::bind_vector<std::vector<GroupObject>>(m, "GroupObjectList");
    
    m.def("Start", &Start, "Start knx handling thread.");
    m.def("ProgramMode", (bool(*)())&ProgramMode, "get programing mode active.");
    m.def("ProgramMode", (bool(*)(bool))&ProgramMode, "Activate / deactivate programing mode.");
    m.def("RegisterGroupObjects", &RegisterGroupObjects);
    
    py::class_<GroupObject>(m, "GroupObject", py::dynamic_attr())
        .def(py::init<uint8_t>())
        .def("objectWrite", (void(GroupObject::*)(float))&GroupObject::objectWrite)
        .def("asap", &GroupObject::asap)
        .def("size", &GroupObject::valueSize)
        .def_property("value",
            [](GroupObject& go) { return py::bytes((const char*)go.valueRef(), go.valueSize()); },
            [](GroupObject& go, py::bytes bytesValue) 
            {
                const auto value = static_cast<std::string>(bytesValue);
                if (value.length() != go.valueSize())
                    throw std::length_error("bytesValue");
            
                auto valueRef = go.valueRef();
                memcpy(valueRef, value.c_str(), go.valueSize());
            })
        .def("callBack", (void(GroupObject::*)(GroupObjectUpdatedHandler))&GroupObject::callback);
}