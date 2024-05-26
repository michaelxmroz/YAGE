#include "Helpers.h"

unsigned int Helpers::GetFirstSetBit(int n)
{
    return static_cast<uint32_t>(log2(n & -n));
}
