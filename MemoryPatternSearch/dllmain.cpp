/*
== PATTERN SEARCH ALGORITHM ==
If you have the byte array of a function you want to find in a game,
or any program by that matter (that doesn't have anti-debugging),
then you can use this function to find the address where that byte array occurs.

I will provide an example of its utility with a "shotbot program".
You can also use this to find other things, like infinite ammo, player health, enemy health, etc.,
as long as you know the byte array of the structure that contains such information.
*/

// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <string>
using namespace std;

// Actual functions bodies are defind at the end of this file.
bool isPattern(const BYTE* pData, const BYTE* bMask, const char* szMask);
DWORD patternSearch(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask);

// Change accordingly:
#define START 0x30000000 // put start address here
#define DELTA 0x00200000 // put the length of search here
#define PATTERN "\x44\x34\x24\x01" // byte array pattern here in this syntax. Can be as long as you want
#define MASK "xxxx" // the mask - make sure there are as many characters in this string as there are bytes in the pattern.
// --> format for the mask: x for what you want to be included in the search, ? for wildcards.
//   --> see the algorithm description for more details down below.

// Example: Say the address we want to find is as above;
//  Then we just bind the address like so:
DWORD address = patternSearch(START, DELTA, (BYTE*)PATTERN, MASK);

// If the address we were looking for was the pointer to a certain structure,
// and we know that the, say, "crosshair" address is offset 0x60 away from it,
// we can find that as well.
DWORD xhair = (DWORD)(address + 0x60);

// In this particular example, we will build a shotbot.
// (When the crosshair aims at a player, it shoots. Automatic firing. You still have to aim, but it shoots for you - instantly.)
// Great for sniping!

// loop control variables...
bool on = false;
int value;

void loop() {
	// This will happen forever while the game is up and running
	// and the dll is injected.
	// This is fine because this program is multi-threaded.

	// The main while loop will check for if the numpad 5 is pressed to turn the shotbot on.
	// The inner while loop will be the actual execution of the shotbot. Within it is an option to turn it off and return to the main while loop.
	while (1) {
		//DWORD xhair = 0x3A5136D0; // if you don't wish to find the address using patternSearch, you can explicitly declare it too using this line (uncommented)
		// --> this should be outside of loop() - I just put it in here to show that if you have the address already, you can dereference and read it just the same ... obviously.


		if (GetAsyncKeyState(VK_NUMPAD5 & 0x8000)) { // turn on by using numpad 5 - 0x8000 is for the downpress (if we don't include it, this could be evaluated twice! once on the rising edge, the other on the falling edge)
			on = true; 
			value = (int)(*(DWORD*)xhair); // the x-hair value

			// begin: debugging purposes to show you the value //
			char buffer[10]; 
			sprintf_s(buffer, "%d", value); 
			MessageBoxA(NULL, buffer, "IT'S ON!", MB_OK); 
			// end: debug //
		}
		while (on) { // now that our shotbot is on,
			value = (int)(*(DWORD*)xhair); // get the value every time this loops (this will be 2 when "aimed", something else when not.)
			if (value == 2) {
				// when the value is 2, we send a mouse click event
				// ... I haven't actually programmed the mouse click event in, so we'll just send a message box popup :)
				// ... think of it as a debugging message.
				MessageBoxA(NULL, "Shoot!", "Shoot!", MB_OK);
			}

			// through each iteration of the "on" loop, we check to see if the hotkey is pressed again to turn it off.
			// if so, we just stop the loop.
			if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000) {
				on = false;
			}
			Sleep(50); // give the processor some time to breath
		}
		Sleep(50);
	}
}

// Main function.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) { // we will be injecting the dll into the process
		MessageBoxA(NULL, "Made by Ayamin.", "Successful injection!", MB_OK); // messagebox showing credits
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&loop, 0, 0, 0); // multi-thread the loop function by passing in its address
	}
	return TRUE;
}


/*
======== Now the main meat of this program: The actual patternsearch algorithm.====
======== I will try my best to explain how it works in the comments. ==============
*/

// isPattern:
// takes an address in memory, a byte mask (the byte array) to find, and a string of tokens denoting which bytes to ignore.
// searches through the byte array to find a match at a ***particular point in memory***

bool isPattern(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask){
		if (*szMask == 'x' && *pData != *bMask) {
			// this guard is true only when we reach a byte we want to evaluate (*szMask == 'x')
			// AND if at this point, the data doesn't match what we are looking for (the data being the instructions at the memory address we are currently at)
			return false; // will only be reached when a non-match occurs (aka, if we match, we keep going)
		}
	}
	return (*szMask) == NULL; // will always return true when all bytes in the byte-array bMask have been exhausted - this will be at the same time that all tokens in szMask have been exhausted
	// i.e., we have compared every single byte in the byte array and it has all matched in the memory.
}

/* takes a start address dwAddress and a length dwLen //
runs a search from the start address for a pattern of bytes, the byte array bMask //
we have a mask that filters out variables (denoted by ?'s) denoted as "wildcards" //
such "wildcards" are not counted in the byte pattern
(i.e., if we have bMask (BYTE*)("\x40\x23\xB9\x1F") and are using the string mask "xx??",
then "\x40\x23\x9F\x26" will still count as a match, because the last two bytes are excluded and considered anomalies)
-- in most games, these will be the bytes that change throughout updates. other bytes will still be consistent in the function we are looking for


=== Complexity remark: operates in O(nm) time in worst case; n is the length of the byte-array (szMask), m is the length of memory we are searching (dwLen) ===

*/

DWORD patternSearch(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char* szMask)
{
	for (DWORD i = 0; i < dwLen; i++) {
		if (isPattern((BYTE*)(dwAddress + i), bMask, szMask)) { // isPattern determines if the instructions that start at a single address matches our array of bytes'
			return (DWORD)(dwAddress + i); // if so, return that address - we have found it!
		}
	}

	return 0; // return 0 if we cannot find the address T_T
}

