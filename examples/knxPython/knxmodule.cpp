#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <Python.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <thread>
#include <stdint.h>
#include <vector>
#include <algorithm>

#include <knx/bits.h>
#include <knx/platform/linux_platform.h>
#include <knx/ip/bau57B0.h>
#include <knx/interface_object/group_object_table_object.h>
#include <knx/util/logger.h>

#define LOGGER Logger::logger("knxmodule")

using namespace Knx;

LinuxPlatform* platform = 0;
Bau57B0* bau = 0;

bool running = false;

static void loop()
{
    while (running)
    {
        bau->loop();
        delayMicroseconds(100);
    }
}

static std::thread workerThread;
static std::vector<std::string> argsVector;
static std::vector<const char*> argv;

struct StdStringCStrFunctor
{
    const char* operator() (const std::string& str)
    {
        return str.c_str();
    }
};

static void init()
{
    Logger::logLevel("knxmodule", Logger::Info);
    Logger::logLevel("ApplicationLayer", Logger::Info);
    Logger::logLevel("BauSystemBDevice", Logger::Info);
    Logger::logLevel("GroupObject", Logger::Info);
    Logger::logLevel("TableObject", Logger::Info);
    Logger::logLevel("Memory", Logger::Info);

    /*
        // copy args so we control the livetime of the char*
        argsVector = args;

        for (int i = 0; i < args.size(); i++)
            printf("%s\n", args[i].c_str());

        argv = std::vector<const char*>(argsVector.size());
        std::transform(argsVector.begin(), argsVector.end(), argv.begin(), StdStringCStrFunctor());
    */
    platform = new LinuxPlatform();
    platform->cmdLineArgs(argv.size(), const_cast<char**>(argv.data()));
    bau = new Bau57B0(*platform);

}

static void Destroy()
{
    delete platform;
    delete bau;
    platform = 0;
    bau = 0;
}

static void ReadMemory()
{
    if (!bau)
        init();

    bau->readMemory();
}

static void Start()
{
    if (running)
        return;

    if (!bau)
        init();

    running = true;

    bau->enabled(true);

    workerThread = std::thread(loop);
    workerThread.detach();
}

static void Stop()
{
    if (!running)
        return;

    running = false;
    bau->writeMemory();
    bau->enabled(false);

    workerThread.join();
}

static bool ProgramMode(bool value)
{
    if (!bau)
        init();

    LOGGER.info("ProgramMode %d", value);
    bau->deviceObject().progMode(value);
    return bau->deviceObject().progMode();
}

static bool ProgramMode()
{
    if (!bau)
        init();

    return bau->deviceObject().progMode();
}

static bool Configured()
{
    if (!bau)
        init();

    return bau->configured();
}


PYBIND11_MODULE(knx, m)
{
    m.doc() = "wrapper for knx device lib";    // optional module docstring

    m.def("Start", &Start, "Start knx handling thread.");
    m.def("Stop", &Stop, "Stop knx handling thread.");
    m.def("Destroy", &Destroy, "Free object allocated objects.");
    m.def("ProgramMode", (bool(*)())&ProgramMode, "get programing mode active.");
    m.def("ProgramMode", (bool(*)(bool))&ProgramMode, "Activate / deactivate programing mode.");
    m.def("Configured", (bool(*)())&Configured, "get configured status.");
    m.def("ReadMemory", &ReadMemory, "read memory from flash file");
    m.def("FlashFilePath", []()
    {
        if (!platform)
            init();

        return platform->flashFilePath();
    });
    m.def("FlashFilePath", [](std::string path)
    {
        if (!platform)
            init();

        platform->flashFilePath(path);
    });
    m.def("GetGroupObject", [](uint16_t goNr)
    {
        LOGGER.info("GetGroupObject arg %d", goNr);
        LOGGER.info("GetGroupObject entrycount %d", bau->groupObjectTable().entryCount());

        if (!bau)
            init();

        if (goNr > bau->groupObjectTable().entryCount())
            return (GroupObject*)nullptr;

        return &bau->groupObjectTable().get(goNr);
    }, py::return_value_policy::reference);
    m.def("Callback", [](GroupObjectUpdatedHandler handler)
    {
        GroupObject::classCallback(handler);
    });
    m.def("Parameters", []()
    {
        uint8_t* data = bau->parameters().data();

        if (data == nullptr)
            return py::bytes();

        return py::bytes((const char*)data, bau->parameters().dataSize());
    });

    py::class_<GroupObject>(m, "GroupObject", py::dynamic_attr())
    .def(py::init())
    .def("asap", &GroupObject::asap)
    .def("size", &GroupObject::valueSize)
    .def_property("value",
                  [](GroupObject & go)
    {

        return py::bytes((const char*)go.valueRef(), go.valueSize());
    },
    [](GroupObject & go, py::bytes bytesValue)
    {
        const auto value = static_cast<std::string>(bytesValue);

        if (value.length() != go.valueSize())
            throw std::length_error("bytesValue");

        auto valueRef = go.valueRef();
        memcpy(valueRef, value.c_str(), go.valueSize());
        go.objectWritten();
    });

}
