#include "Escape.h"

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Draw.h"
#include "KeyControl.h"
#include "Main.h"

int Call_Escape(void)
{
	while (1)
	{
		// Get pressed keys
		GetTrg();
		
		if (gKeyTrg & KEY_ESCAPE) // Escape is pressed, quit game
		{
			gKeyTrg = 0;
			return enum_ESCRETURN_exit;
		}
		if (gKeyTrg & KEY_F1) // F1 is pressed, continue
		{
			gKeyTrg = 0;
			return enum_ESCRETURN_continue;
		}
		if (gKeyTrg & KEY_F2) // F2 is pressed, reset
		{
			gKeyTrg = 0;
			return enum_ESCRETURN_restart;
		}
		
		//Draw screen capture
		PutBitmap4(&grcGame, 0, 0, &grcGame, SURFACE_ID_SCREEN_GRAB);
		
		PutFramePerSecound();
		
		if (!Flip_SystemTask())
		{
			// Quit if window is closed
			gKeyTrg = 0;
			return enum_ESCRETURN_exit;
		}
	}

	return enum_ESCRETURN_exit;
}
