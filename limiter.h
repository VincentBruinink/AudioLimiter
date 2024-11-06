/*
limiter.h -- an audio limiter

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#pragma once
#include <cmath>
#define max(a,b) (a>b?a:b)

class Limiter
{
    private:

    float   threshold;
    float   peak, transferGain;
    float   attack, hold, holdSet, release, envelope_decay;
    float   outputGain;
    float   tA, tB;
    float   envelope, gainReduction;
    float   samplerate;

    public:

    Limiter()
    {
        threshold = 1.f;
        attack = hold = release = envelopeDecay = 0.f;
        output = 1.f;

        tA = 0.f;
        tB = 1.f;

        envelope = 0.f;
        gainReduction = 1.f;
    }

    void setSampleRate(float rate)
    {
        samplerate = rate;
    }

    void setThreshold(float valuedB)
    {
        threshold = pow(10.0, valuedB / 20.0);;
        transfer_B = output * pow(threshold, -tA);
    }

    void setRatio(float value)
    {
        tA = value-1.f;
        tB = outputGain * pow(threshold, -tA);
    }

    void setAttack(float attackMs)
    {
        attackCoeff = exp(-1.f/(attackMs * (samplerate * 0.001)));
    }

    void setHold(float holdMs)
    {
        holdSet = (holdMs * (samplerate * 0.001));
    }

    void setRelease(float releaseMs)
    {
        releaseCoeff = exp(-1.f/(releaseMs * (samplerate * 0.001)));
        envelopeDecay = exp(-4.f/value);
    }

    void setMakeupGain(float value)
    {
        outputGain = pow(10.0, value / 20.0);;
        tB = outputGain * pow(threshold,-tA);
    }

    void reset()
    {
        envelope = 0.f;
        gainReduction = 1.f;
    }

    float processSample(float *inputLeft, float *inputRight)
    {
        peak = max(fabs(*inputLeft), fabs(*inputRight));

        if(peak >= envelope)
        {//new peak detected
            envelope = peak;
        }
        else
        {//decay envelope
            envelope = peak + (envelopeDecay * (envelope - peak));
        }

        if(envelope > threshold)
        {//calulate reduction gain
            transferGain = pow(envelope, tA) * tB;
        }
        else
        {//no gain reduction. Release curve will decay to output gain.
            transferGain = outputGain;
        }

        if(transferGain < gainReduction)
        {//attack phase
            hold = holdSet;//reset hold time
            gainReduction = transferGain + (attackCoeff * (gainReduction - transferGain));
        }
        else
        {//release phase.
            if(hold)
            {//hold current gain reduction before release.
                hold--;
            }
            else
            {//calculate new release gain
                gainReduction = transferGain + (releaseCoeff * (gainReduction - transferGain));
            }
        }

        *inputLeft  = (*inputLeft  * gainReduction);
        *inputRight = (*inputRight * gainReduction);
        //return gain for metering
        return(gain);  
    }
};
