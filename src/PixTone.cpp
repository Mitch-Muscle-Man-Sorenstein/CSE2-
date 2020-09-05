#include "PixTone.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "WindowsWrapper.h"

#include "Attributes.h"
#include "Random.h"

signed char gWaveModelTable[6][0x100];

void MakeWaveTables(void)
{
	int i;
	int a;
	
	// Sine wave
	for (i = 0; i < 0x100; ++i)
	{
		gWaveModelTable[0][i] = (signed char)(sin((i * 6.283184) / 256.0) * 64.0);
		a = gWaveModelTable[0][i];	// I have no idea what this line was meant to do
	}
	
	// Triangle wave
	for (a = 0, i = 0; i < 0x40; ++i)
	{
		// Upwards
		gWaveModelTable[1][i] = (a * 0x40) / 0x40;
		++a;
	}
	for (a = 0; i < 0xC0; ++i)
	{
		// Downwards
		gWaveModelTable[1][i] = 0x40 - ((a * 0x40) / 0x40);
		++a;
	}
	for (a = 0; i < 0x100; ++i)
	{
		// Back up
		gWaveModelTable[1][i] = ((a * 0x40) / 0x40) - 0x40;
		++a;
	}
	
	// Saw up wave
	for (i = 0; i < 0x100; ++i)
		gWaveModelTable[2][i] = (i / 2) - 0x40;
	
	// Saw down wave
	for (i = 0; i < 0x100; ++i)
		gWaveModelTable[3][i] = 0x40 - (i / 2);
	
	// Square wave
	for (i = 0; i < 0x80; ++i)
		gWaveModelTable[4][i] = 0x40;
	for (; i < 0x100; ++i)
		gWaveModelTable[4][i] = -0x40;
	
	// White noise wave
	msvc_srand(0);
	for (i = 0; i < 0x100; ++i)
		gWaveModelTable[5][i] = (signed char)(msvc_rand() & 0xFF) / 2;
}

ATTRIBUTE_HOT BOOL MakePixelWaveData(const PIXTONEPARAMETER *ptp, signed char *pData)
{
	//Construct PixTone waveforms
	static BOOL wave_tables_made = FALSE;
	if (wave_tables_made != TRUE)
	{
		MakeWaveTables();
		wave_tables_made = TRUE;
	}
	
	//Get envelopes
	signed char envelopeTable[0x100];
	memset(envelopeTable, 0x00, sizeof(envelopeTable));
	
	int i = 0;
	double dEnvelope;
	
	//Initial to envelope A
	dEnvelope = ptp->initial;
	while (i < ptp->pointAx)
	{
		envelopeTable[i] = (signed char)dEnvelope;
		dEnvelope = (((double)ptp->pointAy - ptp->initial) / ptp->pointAx) + dEnvelope;
		++i;
	}
	
	//Envelope A to envelope B
	dEnvelope = ptp->pointAy;
	while (i < ptp->pointBx)
	{
		envelopeTable[i] = (signed char)dEnvelope;
		dEnvelope = (((double)ptp->pointBy - ptp->pointAy) / (double)(ptp->pointBx - ptp->pointAx)) + dEnvelope;
		++i;
	}
	
	//Envelope B to envelope C
	dEnvelope = ptp->pointBy;
	while (i < ptp->pointCx)
	{
		envelopeTable[i] = (signed char)dEnvelope;
		dEnvelope = ((double)ptp->pointCy - ptp->pointBy) / (double)(ptp->pointCx - ptp->pointBx) + dEnvelope;
		++i;
	}
	
	//Envelope C to end
	dEnvelope = ptp->pointCy;
	while (i < 0x100)
	{
		envelopeTable[i] = (signed char)dEnvelope;
		dEnvelope = dEnvelope - (ptp->pointCy / (double)(0x100 - ptp->pointCx));
		++i;
	}
	
	//Remember wave offsets
	double dPitch = ptp->oPitch.offset;
	double dMain = ptp->oMain.offset;
	double dVolume = ptp->oVolume.offset;
	
	//Get wave lengths
	double d1, d2, d3;
	if (ptp->oMain.num == 0.0)
		d1 = 0.0;
	else
		d1 = 256.0 / (ptp->size / ptp->oMain.num);
	
	if (ptp->oPitch.num == 0.0)
		d2 = 0.0;
	else
		d2 = 256.0 / (ptp->size / ptp->oPitch.num);
	
	if (ptp->oVolume.num == 0.0)
		d3 = 0.0;
	else
		d3 = 256.0 / (ptp->size / ptp->oVolume.num);
	
	//Synthesize waveform
	for (i = 0; i < ptp->size; ++i)
	{
		int a = (int)dMain % 0x100;
		int b = (int)dPitch % 0x100;
		int c = (int)dVolume % 0x100;
		int d = (int)((double)(i * 0x100) / ptp->size);
		pData[i] = gWaveModelTable[ptp->oMain.model][a]
		         * ptp->oMain.top
		         / 64
		         * (((gWaveModelTable[ptp->oVolume.model][c] * ptp->oVolume.top) / 64) + 64)
		         / 64
		         * envelopeTable[d]
		         / 64;
		
		if (gWaveModelTable[ptp->oPitch.model][b] < 0)
			dMain = d1 - d1 * 0.5 * -gWaveModelTable[ptp->oPitch.model][b] * ptp->oPitch.top / 64.0 / 64.0 + dMain;
		else
			dMain = d1 + d1 * 2.0 * gWaveModelTable[ptp->oPitch.model][b] * ptp->oPitch.top / 64.0 / 64.0 + dMain;
		
		dPitch = dPitch + d2;
		dVolume = dVolume + d3;
	}
	return TRUE;
}
