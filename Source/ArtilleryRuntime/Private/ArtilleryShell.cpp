#include "ArtilleryShell.h"

uint32 FArtilleryShell::GetStickLeftX()
{
    return MyInputActions >> 53;
}

uint32 FArtilleryShell::GetStickLeftY()
{
    return (MyInputActions >> 42) & 0b11111111111;
}

uint32 FArtilleryShell::GetStickRightX()
{
    return (MyInputActions >> 31) & 0b11111111111;
}

uint32 FArtilleryShell::GetStickRightY()
{
    return (MyInputActions >> 20) & 0b11111111111;
}


// index is 0 - 13
bool FArtilleryShell::GetInputAction(int inputActionIndex)
{
    return (MyInputActions >> (19 - inputActionIndex)) & 0b1;
}

// index is 0 - 5 
bool FArtilleryShell::GetEvent(int eventIndex)
{
    return (MyInputActions >> (5 - eventIndex)) & 0b1;
}
/**
* 	std::bitset<11> lx;
	std::bitset<11> ly;
	std::bitset<11> rx;
	std::bitset<11> ry;
	std::bitset<14> buttons;
	std::bitset<6> events;

    sticks: 1 - 44 (1 - 11, 12 - 22, 23 - 33, 34 - 44)
    buttons: 45 - 58
    events: 59 - 64
    sticks: (64 >> 53), (64 >> 42)
    buttons: (64 >> 19) - (64 >> 6)
    events: (64 >> 5) - (64 >> 0)
*/