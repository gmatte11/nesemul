#include "clock.h"
#include "emulator.h"
#include "sfml_renderer.h"
#include "ui/global.h"

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        ui::initialize();

        Emulator emul;
        SFMLRenderer renderer(&emul);

        Timer timer(sixtieth_us);

        for (;;)
        {
            if (timer.expired())
            {
                emul.update();
                if (!renderer.update())
                    break;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
