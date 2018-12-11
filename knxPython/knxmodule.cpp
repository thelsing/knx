#include <Python.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include "linux_platform.h"
#include "knx/bau57B0.h"
#include "knx/group_object_table_object.h"


LinuxPlatform platfrom;
Bau57B0 bau(platfrom);

#if 1 // this code will go to python later
float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
long lastsend = 0;

GroupObject groupObjects[]
{
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(1)
}
;
#define CURR groupObjects[0]
#define MAX groupObjects[1]
#define MIN groupObjects[2]
#define RESET groupObjects[3]

void measureTemp()
{
    long now = platfrom.millis();
    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;
    
    CURR.objectWrite(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        MAX.objectWrite(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        MIN.objectWrite(minValue);
    }
}

void resetCallback(GroupObject& go)
{
    if (go.objectReadBool())
    {
        maxValue = 0;
        minValue = 10000;
    }
}

void appLoop()
{
    if (!bau.configured())
        return;
    
    measureTemp();
}

void setup()
{
    srand((unsigned int)time(NULL));
    bau.readMemory();
    
    GroupObjectTableObject& got(bau.groupObjectTable());
    got.groupObjects(groupObjects, 4);
    
    DeviceObject& devObj(bau.deviceObject());
    devObj.manufacturerId(0xfa);

    RESET.updateHandler = resetCallback;


    if (bau.parameters().loadState() == LS_LOADED)
    {
        printf("Timeout: %d\n", bau.parameters().getWord(0));
        printf("Zykl. senden: %d\n", bau.parameters().getByte(2));
        printf("Min/Max senden: %d\n", bau.parameters().getByte(3));
        printf("Aenderung senden: %d\n", bau.parameters().getByte(4));
        printf("Abgleich %d\n", bau.parameters().getByte(5));
    }
    bau.enabled(true);
}
#endif

static void* loop(void* x)
{
    while (1)
    {
        bau.loop();
        appLoop();
        platfrom.mdelay(100);
    }
        
    pthread_exit(NULL);
}

static pthread_t workerThread;

static PyObject* Start(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    setup();

    int res = pthread_create(&workerThread, NULL, loop, NULL);
    if (res)
    {
        printf("error %d\n", res);
        return NULL;
    }
    
    Py_RETURN_NONE;
}

static PyObject* ProgramMode(PyObject* self, PyObject* args)
{
    if (PyArg_ParseTuple(args, ""))
    {
        //no arguments
        return Py_BuildValue("i", bau.deviceObject().progMode());
    }
    PyErr_Clear();
    int value = 0;
    if (!PyArg_ParseTuple(args, "i", &value))
        return NULL;
    bau.deviceObject().progMode(value);
    return Py_BuildValue("i", bau.deviceObject().progMode());
}

static PyMethodDef knxMethods[] = 
{
    {"Start", Start, METH_VARARGS, "Start knx handling thread." },
	{"ProgramMode", ProgramMode, METH_VARARGS, "Activate/deactivate programing mode." },	
    { NULL, NULL, 0, NULL }        /* End of list */
};


static struct PyModuleDef knxModule = {
    PyModuleDef_HEAD_INIT,
    "knx",
    NULL,
    0,
    knxMethods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC PyInit_knx(void)
{
    PyObject* m = PyModule_Create(&knxModule);
    return m;
}

