#include "global.h"

struct Globals
{
    bool init_ = false;
    sf::Font font_;

    static Globals instance_;
    static Globals& get() { return instance_; }
};

Globals Globals::instance_;

void ui::initialize()
{
    Globals& g = Globals::get();
    
    if (!g.init_)
    {
        g.init_ = true;
        g.font_.loadFromFile("data/Emulogic-zrEw.ttf");
    }
}

sf::Font const& ui::get_font()
{
    return Globals::get().font_;
}