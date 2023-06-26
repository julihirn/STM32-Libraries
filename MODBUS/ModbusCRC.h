#include "inttypes.h"

namespace Modbus{
    uint16_t CalculateCRC16(uint8_t * Frame, uint16_t Length);
};