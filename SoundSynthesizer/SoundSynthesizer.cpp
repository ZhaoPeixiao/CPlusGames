#include <iostream>
using namespace std;

#include"olcNoiseMaker.h"

// Global synthesizer variables
atomic<double> dFrequencyOutput = 0.0;			// dominant output frequency of instrument, i.e. the note
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve


double MakeNoise(double dTime)
{
    double dOutput = sin(dFrequencyOutput * 2.0 * 3.14159 * dTime);
    return dOutput * 0.5;       // Master Volume
}

int main()
{
    // Get all sound hardware
    vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

    // Display findings
    //for (auto d : devices)
    //{
    //    wcout << "Found Output Device: " << d << endl;
    //}

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
                nCurrentKey = -1;
            }
        }
    }

    return 0;
}
