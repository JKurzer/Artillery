#include "ArtilleryShell.h"

float FArtilleryShell::GetStickLeftX()
{
    return FCableInputPacker::UnpackStick(MyInputActions >> 53);
}

float FArtilleryShell::GetStickLeftY()
{
    return FCableInputPacker::UnpackStick((MyInputActions >> 42) & 0b11111111111);
}

float FArtilleryShell::GetStickRightX()
{
    return FCableInputPacker::UnpackStick((MyInputActions >> 31) & 0b11111111111);
}

float FArtilleryShell::GetStickRightY()
{
    return FCableInputPacker::UnpackStick((MyInputActions >> 20) & 0b11111111111);
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

uint32 FArtilleryShell::GetButtonsAndEventsFlat()
{                         //0b1111 1111 1111 1111 1111 is 20 bits.
    return MyInputActions & 0b11111111111111111111;
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