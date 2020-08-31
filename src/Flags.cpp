#include "Flags.h"

#include <string.h>

#include "WindowsWrapper.h"

// Macros for setting, un-setting and getting flags
// Each flag is stored in a bit, so we can use the exact same macros we'd use for bits
#define SET_FLAG(x, i) ((x)[(i) / 8] |= 1 << ((i) % 8))
#define UNSET_FLAG(x, i) ((x)[(i) / 8] &= ~(1 << ((i) % 8)))
#define GET_FLAG(x, i) ((x)[(i) / 8] & (1 << ((i) % 8)))

unsigned char gFlagNPC[(NPC_FLAG_NUM + 7) / 8];
unsigned char gSkipFlag[(SKIP_FLAG_NUM+7) / 8];

// Flag initializers
void InitFlags(void)
{
	memset(gFlagNPC, 0, sizeof(gFlagNPC));
}

void InitSkipFlags(void)
{
	memset(gSkipFlag, 0, sizeof(gSkipFlag));
}

// NPC flags
void SetNPCFlag(long a)
{
	if (a >= 0 && a < NPC_FLAG_NUM)
		SET_FLAG(gFlagNPC, a);
}

void CutNPCFlag(long a)
{
	if (a >= 0 && a < NPC_FLAG_NUM)
		UNSET_FLAG(gFlagNPC, a);
}

BOOL GetNPCFlag(long a)
{
	if ((a >= 0 && a < NPC_FLAG_NUM) && GET_FLAG(gFlagNPC, a))
		return TRUE;
	else
		return FALSE;
}

// Skip flags
void SetSkipFlag(long a)
{
	if (a >= 0 && a < SKIP_FLAG_NUM)
		SET_FLAG(gSkipFlag, a);
}

void CutSkipFlag(long a)
{
	if (a >= 0 && a < SKIP_FLAG_NUM)
		UNSET_FLAG(gSkipFlag, a);
}

BOOL GetSkipFlag(long a)
{
	if ((a >= 0 && a < SKIP_FLAG_NUM) && GET_FLAG(gSkipFlag, a))
		return TRUE;
	else
		return FALSE;
}
