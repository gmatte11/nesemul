#include "clock.h"
#include "emulator.h"
#include "platform/platform_defines.h"
#include "sfml_renderer.h"
#include "ui/global.h"

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        threading::set_thread_high_priority();
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

            time_unit remaining_ms = std::max(((sixtieth_us - timer.elapsed_us()) / 1000) - 1, 0ll);
            threading::sleep_ms(remaining_ms);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
