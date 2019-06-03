#include <GDBStub.h>

//Comment out the definition below if you don't want to use the ESP8266 gdb stub.
#define ESP8266_USE_GDB_STUB

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
    
#ifdef ESP8266_USE_GDB_STUB
#if !defined(GDBSTUB_BREAK_ON_INIT) || !GDBSTUB_BREAK_ON_INIT
	printf("WARNING! The ESP8266 GDB stub will not wait for gdb during startup. Please add GDBSTUB_BREAK_ON_INIT=1 to Extra Preprocessor Macros.\n");
#endif
    gdbstub_init();
#endif
}

void loop()
{
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
}
