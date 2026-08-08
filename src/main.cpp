#include "application/Application.h"

#ifdef PLATFORM_ESP32

Application app;

void setup()
{
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