#include "Main.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>

#include "WindowsWrapper.h"

#include "Backends/Misc.h"
#include "Bitmap.h"
#include "CommonDefines.h"
#include "Config.h"
#include "Draw.h"
#include "Game.h"
#include "Generic.h"
#include "Input.h"
#include "KeyControl.h"
#include "MyChar.h"
#include "Music.h"
#include "Sound.h"
#include "Triangle.h"
#include "File.h"
#include "Filesystem.h"

GameSeason g_GameSeason;

BOOL bFullscreen;
BOOL gbUseJoystick = FALSE;

int gJoystickButtonTable[8];

static BOOL bActive = TRUE;
static BOOL bFPS = FALSE;

static int windowWidth;
static int windowHeight;

static const char* const lpWindowName = "CSE2-";

// Framerate stuff
static unsigned long CountFramePerSecound(void)
{
	unsigned long current_tick;	// The original name for this variable is unknown
	static BOOL first = TRUE;
	static unsigned long max_count;
	static unsigned long count;
	static unsigned long wait;

	if (first)
	{
		wait = Backend_GetTicks();
		first = FALSE;
	}

	current_tick = Backend_GetTicks();
	++count;

	if (wait + 1000 <= current_tick)
	{
		wait += 1000;
		max_count = count;
		count = 0;
	}

	return max_count;
}

void PutFramePerSecound(void)
{
	if (bFPS)
	{
		const unsigned long fps = CountFramePerSecound();
		PutNumber4(WINDOW_WIDTH - 40, 8, fps, FALSE);
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	
	// Initialize backend
	if (!Backend_Init())
		return EXIT_FAILURE;
	
	// Get game season
	time_t now;
	time(&now);
	struct tm *timeinfo = localtime(&now);
	
	switch (1 + timeinfo->tm_mon)
	{
		case 10:
			g_GameSeason = GS_Halloween;
			break;
		case 12:
			g_GameSeason = GS_Christmas;
			break;
		case 4:
			if (timeinfo->tm_mday == 29)
			{
				g_GameSeason = GS_Pixel;
				break;
			}
	//Fallthrough
		default:
			g_GameSeason = GS_None;
			break;
	}
	
	// Get executable's path
	std::string modulePath;
	if (!Backend_GetBasePath(&modulePath))
	{
		// Fall back on argv[0] if the backend cannot provide a path
		modulePath = argv[0];
		size_t last_pos = modulePath.find_last_of("/\\");
		if (last_pos != std::string::npos)
			modulePath = modulePath.substr(0, last_pos + 1);
		else
			modulePath = ".";
	}
	
	// Initialize filesystem
	if (!Filesystem_Init(modulePath))
		return EXIT_FAILURE;
	
	// Load configuration
	LoadConfigData();
	
	// Apply configuration
	SetMusicType(gConfig.music_type);
	
	// TEMP: Apply keybinds
	gKeyJump = KEY_Z;
	gKeyShot = KEY_X;
	gKeyOk = gKeyJump;
	gKeyCancel = gKeyShot;
	gKeyLeft = KEY_LEFT;
	gKeyUp = KEY_UP;
	gKeyRight = KEY_RIGHT;
	gKeyDown = KEY_DOWN;
	
	// Initialize renderer
	if (!StartDirectDraw(lpWindowName, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2, FALSE))
		return EXIT_FAILURE;
	
	// Load and use window icon
	FILE *window_icon_resource = OpenFile(FSS_Module, "data/icon.bmp", "rb");
	if (window_icon_resource)
	{
		fseek(window_icon_resource, 0, SEEK_END);
		size_t window_icon_resource_size = ftell(window_icon_resource);
		fseek(window_icon_resource, 0, SEEK_SET);
		
		uint8_t *window_icon_resource_data = new uint8_t[window_icon_resource_size];
		fread(window_icon_resource_data, window_icon_resource_size, 1, window_icon_resource);
		
		if (window_icon_resource_data != NULL)
		{
			unsigned int window_icon_width, window_icon_height;
			unsigned char *window_icon_rgb_pixels = DecodeBitmap(window_icon_resource_data, window_icon_resource_size, &window_icon_width, &window_icon_height);
			
			if (window_icon_rgb_pixels != NULL)
			{
				Backend_SetWindowIcon(window_icon_rgb_pixels, window_icon_width, window_icon_height);
				FreeBitmap(window_icon_rgb_pixels);
			}
		}
		
		fclose(window_icon_resource);
		delete[] window_icon_resource_data;
	}
	
	// Set rects
	RECT rcLoading = {0, 0, 64, 8};
	RECT rcFull = {0, 0, 0, 0};
	rcFull.right = WINDOW_WIDTH;
	rcFull.bottom = WINDOW_HEIGHT;
	
	// Load the "LOADING" text
	BOOL b = MakeSurface_File("Loading", SURFACE_ID_LOADING);
	
	// Draw loading screen
	CortBox(&rcFull, 0x000000);
	PutBitmap3(&rcFull, (WINDOW_WIDTH / 2) - 32, (WINDOW_HEIGHT / 2) - 4, &rcLoading, SURFACE_ID_LOADING);
	
	// Draw to screen
	if (!Flip_SystemTask())
	{
		Backend_Deinit();
		return EXIT_SUCCESS;
	}
	
	// Initialize sound
	InitDirectSound();
	
	// Initialize font and trig
	InitTextObject("csfont.fnt");
	InitTriangleTable();
	
	// Enter game loop
	Game();
	
	// Save configuration
	SaveConfigData();
	
	// Free font
	EndTextObject();
	
	// End drawing, sound, and backend
	EndDirectSound();
	EndDirectDraw();
	Backend_Deinit();
	return EXIT_SUCCESS;
}

void InactiveWindow(void)
{
	if (bActive)
		bActive = FALSE;
}

void ActiveWindow(void)
{
	if (!bActive)
		bActive = TRUE;
}

void JoystickProc(void);

BOOL SystemTask(void)
{
	static bool previous_keyboard_state[BACKEND_KEYBOARD_TOTAL];

	if (!Backend_SystemTask())
		return FALSE;

	bool keyboard_state[BACKEND_KEYBOARD_TOTAL];
	Backend_GetKeyboardState(keyboard_state);

	for (unsigned int i = 0; i < BACKEND_KEYBOARD_TOTAL; ++i)
	{
		if (keyboard_state[i] && !previous_keyboard_state[i])
		{
			switch (i)
			{
				case BACKEND_KEYBOARD_ESCAPE:
					gKeyInternal |= KEY_ESCAPE;
					break;

				case BACKEND_KEYBOARD_W:
					gKeyInternal |= KEY_MAP;
					break;

				case BACKEND_KEYBOARD_LEFT:
					gKeyInternal |= KEY_LEFT;
					break;

				case BACKEND_KEYBOARD_RIGHT:
					gKeyInternal |= KEY_RIGHT;
					break;

				case BACKEND_KEYBOARD_UP:
					gKeyInternal |= KEY_UP;
					break;

				case BACKEND_KEYBOARD_DOWN:
					gKeyInternal |= KEY_DOWN;
					break;

				case BACKEND_KEYBOARD_X:
					gKeyInternal |= KEY_X;
					break;

				case BACKEND_KEYBOARD_Z:
					gKeyInternal |= KEY_Z;
					break;

				case BACKEND_KEYBOARD_S:
					gKeyInternal |= KEY_ARMS;
					break;

				case BACKEND_KEYBOARD_A:
					gKeyInternal |= KEY_ARMSREV;
					break;

				case BACKEND_KEYBOARD_LEFT_SHIFT:
				case BACKEND_KEYBOARD_RIGHT_SHIFT:
					gKeyInternal |= KEY_SHIFT;
					break;

				case BACKEND_KEYBOARD_F1:
					gKeyInternal |= KEY_F1;
					break;

				case BACKEND_KEYBOARD_F2:
					gKeyInternal |= KEY_F2;
					break;

				case BACKEND_KEYBOARD_Q:
					gKeyInternal |= KEY_ITEM;
					break;

				case BACKEND_KEYBOARD_COMMA:
					gKeyInternal |= KEY_ALT_LEFT;
					break;

				case BACKEND_KEYBOARD_PERIOD:
					gKeyInternal |= KEY_ALT_DOWN;
					break;

				case BACKEND_KEYBOARD_FORWARD_SLASH:
					gKeyInternal |= KEY_ALT_RIGHT;
					break;

				case BACKEND_KEYBOARD_L:
					gKeyInternal |= KEY_L;
					break;

				case BACKEND_KEYBOARD_EQUALS:
					gKeyInternal |= KEY_PLUS;
					break;

				case BACKEND_KEYBOARD_F5:
					gbUseJoystick = FALSE;
					break;
			}
		}
		else if (!keyboard_state[i] && previous_keyboard_state[i])
		{
			switch (i)
			{
				case BACKEND_KEYBOARD_ESCAPE:
					gKeyInternal &= ~KEY_ESCAPE;
					break;

				case BACKEND_KEYBOARD_W:
					gKeyInternal &= ~KEY_MAP;
					break;

				case BACKEND_KEYBOARD_LEFT:
					gKeyInternal &= ~KEY_LEFT;
					break;

				case BACKEND_KEYBOARD_RIGHT:
					gKeyInternal &= ~KEY_RIGHT;
					break;

				case BACKEND_KEYBOARD_UP:
					gKeyInternal &= ~KEY_UP;
					break;

				case BACKEND_KEYBOARD_DOWN:
					gKeyInternal &= ~KEY_DOWN;
					break;

				case BACKEND_KEYBOARD_X:
					gKeyInternal &= ~KEY_X;
					break;

				case BACKEND_KEYBOARD_Z:
					gKeyInternal &= ~KEY_Z;
					break;

				case BACKEND_KEYBOARD_S:
					gKeyInternal &= ~KEY_ARMS;
					break;

				case BACKEND_KEYBOARD_A:
					gKeyInternal &= ~KEY_ARMSREV;
					break;

				case BACKEND_KEYBOARD_LEFT_SHIFT:
				case BACKEND_KEYBOARD_RIGHT_SHIFT:
					gKeyInternal &= ~KEY_SHIFT;
					break;

				case BACKEND_KEYBOARD_F1:
					gKeyInternal &= ~KEY_F1;
					break;

				case BACKEND_KEYBOARD_F2:
					gKeyInternal &= ~KEY_F2;
					break;

				case BACKEND_KEYBOARD_Q:
					gKeyInternal &= ~KEY_ITEM;
					break;

				case BACKEND_KEYBOARD_COMMA:
					gKeyInternal &= ~KEY_ALT_LEFT;
					break;

				case BACKEND_KEYBOARD_PERIOD:
					gKeyInternal &= ~KEY_ALT_DOWN;
					break;

				case BACKEND_KEYBOARD_FORWARD_SLASH:
					gKeyInternal &= ~KEY_ALT_RIGHT;
					break;

				case BACKEND_KEYBOARD_L:
					gKeyInternal &= ~KEY_L;
					break;

				case BACKEND_KEYBOARD_EQUALS:
					gKeyInternal &= ~KEY_PLUS;
					break;
			}
		}
	}

	memcpy(previous_keyboard_state, keyboard_state, sizeof(keyboard_state));

	// Run joystick code
	if (gbUseJoystick)
		JoystickProc();

	return TRUE;
}

void JoystickProc(void)
{
	int i;
	JOYSTICK_STATUS status;

	if (!GetJoystickStatus(&status))
		return;

	gKeyInternal &= (KEY_ESCAPE | KEY_F1 | KEY_F2);

	// Set movement buttons
	if (status.bLeft)
		gKeyInternal |= gKeyLeft;
	else
		gKeyInternal &= ~gKeyLeft;

	if (status.bRight)
		gKeyInternal |= gKeyRight;
	else
		gKeyInternal &= ~gKeyRight;

	if (status.bUp)
		gKeyInternal |= gKeyUp;
	else
		gKeyInternal &= ~gKeyUp;

	if (status.bDown)
		gKeyInternal |= gKeyDown;
	else
		gKeyInternal &= ~gKeyDown;

	// Clear held buttons
	for (i = 0; i < 8; ++i)
		gKeyInternal &= ~gJoystickButtonTable[i];

	// Set held buttons
	for (i = 0; i < 8; ++i)
		if (status.bButton[i])
			gKeyInternal |= gJoystickButtonTable[i];
}
