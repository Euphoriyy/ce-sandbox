#include <cstdint>

struct Timing
{
    uint24_t frame = 0;
    uint8_t frametime = 0;
};

extern Timing timing;
