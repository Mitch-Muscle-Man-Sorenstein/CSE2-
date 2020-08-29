#include "KeyControl.h"

long gKeyInternal;
long gKey;
long gKeyTrg;

long gKeyJump = KEY_Z;
long gKeyShot = KEY_X;
long gKeyArms = KEY_ARMS;
long gKeyArmsRev = KEY_ARMSREV;
long gKeyItem = KEY_ITEM;
long gKeyMap = KEY_MAP;

long gKeyOk = KEY_Z;
long gKeyCancel = KEY_X;

long gKeyLeft = KEY_LEFT;
long gKeyUp = KEY_UP;
long gKeyRight = KEY_RIGHT;
long gKeyDown = KEY_DOWN;

void GetTrg(void)
{
	static int key_old;
	gKey = gKeyInternal;
	if ((gKey & (gKeyLeft | gKeyRight)) == (gKeyLeft | gKeyRight))
		gKey &= ~(gKeyLeft | gKeyRight);
	if ((gKey & (gKeyUp | gKeyDown)) == (gKeyUp | gKeyDown))
		gKey &= ~(gKeyUp | gKeyDown);
	gKeyTrg = gKey ^ key_old;
	gKeyTrg = gKey & gKeyTrg;
	key_old = gKey;
}
