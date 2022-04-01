// written by Doug Fraker 2019-2020
// NES zapper gun code

// pass it 0 or 1, which port
// returns 0 or 1 if trigger pulled
unsigned char __fastcall__ zap_shoot(unsigned char pad);


// pass it 0 or 1, which port
// returns 0 or 1 if detects light this frame
unsigned char __fastcall__ zap_read(unsigned char pad);


// alt. version from "Falling Tiles" game.
// always reads from port 2
// pass it 1-224, NOT zero
// = approximately how many scanlines to keep reading
// note, PAL will need a little more than NTSC
// allows you to avoid lag frames on misses.
// "Falling Tiles" used 196 for shooting low objects
// and 176 for shooting objects that are higher
// and should exit this loop before the end of the frame.

// returns 0 or 1 if detects light this frame
unsigned char __fastcall__ zap_read2(unsigned char pad);
	
	
	
	