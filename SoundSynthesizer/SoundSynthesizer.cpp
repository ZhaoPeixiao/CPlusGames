#include <list>
#include <iostream>
#include <algorithm>
using namespace std;

#include"olcNoiseMaker.h"
#define FTYPE double

namespace synth
{
	//////////////////////////////////////////////////////////////////////////////
	// Utilities

	// Converts frequency (Hz) to angular velocity
	FTYPE w(const FTYPE dHertz)
	{
		return dHertz * 2.0 * PI;
	}

	// A basic note
	struct note
	{
		int id;			// Position in scale
		FTYPE on;		// Time note was activated
		FTYPE off;		// Time note was deactivated
		bool active;
		int channel;

		note()
		{
			id = 0;
			on = 0.0;
			off = 0.0;
			active = false;
			channel = 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////////
	// Multi-Function Oscillator
	const int OSC_SINE = 0;
	const int OSC_SQUARE = 1;
	const int OSC_TRIANGLE = 2;
	const int OSC_SAW_ANA = 3;
	const int OSC_SAW_DIG = 4;
	const int OSC_NOISE = 5;

	FTYPE osc(const FTYPE dTime, const FTYPE dHertz, const int nType = OSC_SINE,
		const FTYPE dLFOHertz = 0.0, const FTYPE dLFOAmplitude = 0.0, FTYPE dCustom = 50.0)
	{
		FTYPE dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * (sin(w(dLFOHertz) * dTime));// osc(dTime, dLFOHertz, OSC_SINE);

		switch (nType)
		{
		case OSC_SINE: // Sine wave bewteen -1 and +1
			return sin(dFreq);

		case OSC_SQUARE: // Square wave between -1 and +1
			return sin(dFreq) > 0 ? 1.0 : -1.0;

		case OSC_TRIANGLE: // Triangle wave between -1 and +1
			return asin(sin(dFreq)) * (2.0 / PI);

		case OSC_SAW_ANA: // Saw wave (analogue / warm / slow)
		{
			FTYPE dOutput = 0.0;
			for (FTYPE n = 1.0; n < dCustom; n++)
				dOutput += (sin(n * dFreq)) / n;
			return dOutput * (2.0 / PI);
		}

		case OSC_SAW_DIG:
			return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

		case OSC_NOISE:
			return 2.0 * ((FTYPE)rand() / (FTYPE)RAND_MAX) - 1.0;

		default:
			return 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Scale to Frequency conversion
	const int SCALE_DEFAULT = 0;

	FTYPE scale(const int nNoteID, const int nScaleID = SCALE_DEFAULT)
	{
		switch(nScaleID)
		{
		case SCALE_DEFAULT: default:
			return 256 * pow(1.0594630943592952645618252949463, nNoteID);
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Envelopes
	struct envelope
	{
		virtual FTYPE amplitude(const FTYPE dTime, const FTYPE dTimeOn, const FTYPE dTimeOff) = 0;
	};


}

















// General purpose oscillator
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_DIG 4
#define OSC_NOISE 5

double osc(double dHertz, double dTime, int nType = OSC_SINE, double dLFOHertz = 0.0, double dLFOAmplitude = 0.0)
{
	double dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * sin(w(dLFOHertz) * dTime);

	switch (nType)
	{
	case OSC_SINE: // Sine wave bewteen -1 and +1
		return sin(dFreq);

	case OSC_SQUARE: // Square wave between -1 and +1
		return sin(dFreq * dTime) > 0 ? 1.0 : -1.0;

	case OSC_TRIANGLE: // Triangle wave between -1 and +1
		return asin(sin(dFreq)) * (2.0 / PI);

	case OSC_SAW_ANA: // Saw wave (analogue / warm / slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 40.0; n++)
			dOutput += (sin(n * dFreq)) / n;
		return dOutput * (2.0 / PI);
	}

	case OSC_SAW_DIG: // Saw Wave (optimised / harsh / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

	case OSC_NOISE: // Pseudorandom noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;

	default:
		return 0.0;
	}
}

struct sEnvelopeADSR
{
	double dAttackTime;
	double dDecayTime;
	double dSustainAmplitude;
	double dReleaseTime;
	double dStartAmplitude;
	double dTriggerOffTime;
	double dTriggerOnTime;
	bool bNoteOn;

	sEnvelopeADSR()
	{
		dAttackTime = 0.01;
		dDecayTime = 1.0;
		dStartAmplitude = 1.0;
		dSustainAmplitude = 0.0;
		dReleaseTime = 1.0;
		bNoteOn = false;
		dTriggerOffTime = 0.0;
		dTriggerOnTime = 0.0;
	}

	double GetAmplitude(double dTime)
	{
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime;

		if (bNoteOn)
		{
			// ADS

			// Attack
			if (dLifeTime <= dAttackTime)
			{
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
			}

			// Decay
			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
			{
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;
			}

			// Sustain
			if (dLifeTime > (dAttackTime + dDecayTime))
			{
				// In sustain phase - dont change until note released
				dAmplitude = dSustainAmplitude;
			}
		}
		else
		{
			// R
			// Note has been released, so in release phase
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;

			// Amplitude should not be negative
			if (dAmplitude <= 0.0001)
				dAmplitude = 0.0;

			return dAmplitude;
		}
		return dAmplitude;
	}

	void NoteOn(double dTimeOn)
	{
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteOff(double dTimeOff)
	{
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}
};


struct instrument
{
	double dVolume;
	sEnvelopeADSR env;

	virtual double sound(double dTime, double dFrequency) = 0;
};


struct bell : public instrument
{
	bell()
	{
		env.dAttackTime = 0.01;
		env.dDecayTime = 1.0;
		env.dStartAmplitude = 1.0;
		env.dSustainAmplitude = 0.0;
		env.dReleaseTime = 1.0;
	}

	double sound(double dTime, double dFrequency)
	{
		double dOutput = envelope.GetAmplitude(dTime) *
			(
				+1.0 * osc(dFrequencyOutput * 2.0, dTime, OSC_SINE, 5.0, 0.01)
				+ 0.5 * osc(dFrequencyOutput * 3.0, dTime, OSC_SINE)
				+ 0.25 * osc(dFrequencyOutput * 4.0, dTime, OSC_SINE)
				);

		return dOutput;
	}
};


// Global synthesizer variables
atomic<double> dFrequencyOutput = 0.0;			// dominant output frequency of instrument, i.e. the note
sEnvelopeADSR envelope;
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve

instrument* voice = nullptr;


double MakeNoise(int nChannel, double dTime)
{
	double dOutput = voice->sound(dTime, dFrequencyOutput);
	return dOutput * 0.4;	// Master Volumn
}

int main()
{
	// Shameless self-promotion
	wcout << "Multiple Oscillators with Single Amplitude Envelope, No Polyphony" << endl << endl;

	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	//for (auto d : devices)
	//{
	//	wcout << "Found Output Device: " << d << endl;
	//}
	wcout << "Using Device: " << devices[0] << endl;

	// Display a keyboard
	wcout << endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	// Create sound machine!!
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	voice = new bell();
	sound.SetUserFunction(MakeNoise);

	// Sit in loop, capturing keyboard state changes and modify
	// synthesizer output accordingly
	int nCurrentKey = -1;
	bool bKeyPressed = false;
	while (1)
	{
		bKeyPressed = false;
		for (int k = 0; k < 16; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
			{
				if (nCurrentKey != k)
				{
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
					voice->env.NoteOn(sound.GetTime());
					wcout << "\rNote On : " << sound.GetTime() << "s " << dFrequencyOutput << "Hz";
					nCurrentKey = k;
				}
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed)
		{
			if (nCurrentKey != -1)
			{
				wcout << "\rNote Off: " << sound.GetTime() << "s                        ";
				voice->env.NoteOff(sound.GetTime());
				nCurrentKey = -1;
			}
		}
	}

	return 0;
}