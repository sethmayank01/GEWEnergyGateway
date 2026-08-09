#include "application/Application.h"

#ifdef PLATFORM_ESP32
#include <Arduino.h>
Application app;

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        delay(10);
    app.Run();
}

void loop()
{
    // Never reached because Gateway::Run() loops forever.
}

#else

int main()
{
    Application app;
    return app.Run();
}

#endif