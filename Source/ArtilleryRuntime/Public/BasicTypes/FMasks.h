// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


namespace Arty
{
	namespace Intents{	
		//Buttons, please read:
		//1 bit per button
		//in LOWEST TO HIGHEST order
		// Menu ,
		// View 
		// A,
		// B,
		// X,
		// Y,
		// DPadUp,
		// DPadDown,
		// DPadLeft,
		// DPadRight,
		// LeftShoulder,
		// RightShoulder,
		// LeftTrigger,
		// RightTrigger,
		typedef uint64 Intent;
		constexpr const size_t TYPEBREAK_MAPPING_FROM_BC_BUTTONS = 14;
		constexpr const size_t TYPEBREAK_MAPPING_FROM_BC_EVENTS = 6;
		constexpr Intent Menu = 0b1;
		constexpr Intent View = 0b10;
		constexpr Intent A = 0b100;
		constexpr Intent B = 0b1000;
		constexpr Intent X = 0b10000;
		constexpr Intent Y = 0b100000;
		constexpr Intent DUp = 0b1000000;
		constexpr Intent DDown = 0b10000000;
		constexpr Intent DLeft = 0b100000000;
		constexpr Intent DRight = 0b1000000000;
		constexpr Intent LShoulder = 0b10000000000;
		constexpr Intent RShoulder = 0b100000000000;
		constexpr Intent LTrigger = 0b1000000000000;
		constexpr Intent RTrigger = 0b10000000000000;

		constexpr int MenuIndex = 0;
		constexpr int ViewIndex = 1;
		constexpr int AIndex = 2;
		constexpr int BIndex = 3;
		constexpr int XIndex = 4;
		constexpr int YIndex = 5;
		constexpr int DUpIndex = 6;
		constexpr int DDownIndex = 7;
		constexpr int DLeftIndex = 8;
		constexpr int DRightIndex = 9;
		constexpr int LShoulderIndex = 10;
		constexpr int RShoulderIndex = 11;
		constexpr int LTriggerIndex = 12;
		constexpr int RTriggerIndex = 13;


		constexpr int Event1Index = 0;
		constexpr int Event2Index = 1;
		constexpr int Event3Index = 2;
		constexpr int Event4Index = 3;
		constexpr int Event5Index = 4;
		constexpr int Event6Index = 5;
	};
}