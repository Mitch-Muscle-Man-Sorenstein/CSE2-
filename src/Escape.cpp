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
		
		//Exit if escape is pressed again
		if (gKeyTrg & KEY_ESCAPE)
		{
			gKeyInternal &= ~KEY_ESCAPE;
			gKeyTrg = 0;
			return enum_ESCRETURN_continue;
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

	return enum_ESCRETURN_continue;
}
