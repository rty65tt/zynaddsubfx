/*
  ZynAddSubFX - a software synthesizer

  Echo.cpp - Echo effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2009-2010 Mark McCurry
  Author: Nasca Octavian Paul
          Mark McCurry

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <cmath>
#include "Echo.h"

#define MAX_DELAY 2

Echo::Echo(const int &insertion_,
           REALTYPE *const efxoutl_,
           REALTYPE *const efxoutr_)
    :Effect(insertion_, efxoutl_, efxoutr_, NULL, 0),
      Pvolume(50), Ppanning(64), Pdelay(60),
      Plrdelay(100), Plrcross(100), Pfb(40), Phidamp(60),
      delayTime(1), lrdelay(0), avgDelay(0),
      delay(new REALTYPE[(int)(MAX_DELAY * SAMPLE_RATE)],
            new REALTYPE[(int)(MAX_DELAY * SAMPLE_RATE)]),
      old(0.0), pos(0), delta(1), ndelta(1)
{
    initdelays();
    setpreset(Ppreset);
}

Echo::~Echo()
{
    delete[] delay.l();
    delete[] delay.r();
}

/*
 * Cleanup the effect
 */
void Echo::cleanup()
{
    memset(delay.l(),0,MAX_DELAY*SAMPLE_RATE*sizeof(REALTYPE));
    memset(delay.r(),0,MAX_DELAY*SAMPLE_RATE*sizeof(REALTYPE));
    old = Stereo<REALTYPE>(0.0);
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

/*
 * Initialize the delays
 */
void Echo::initdelays()
{
    cleanup();
    //number of seconds to delay left chan
    float dl = avgDelay - lrdelay;

    //number of seconds to delay right chan
    float dr = avgDelay + lrdelay;

    ndelta.l() = max(1,(int) (dl * SAMPLE_RATE));
    ndelta.r() = max(1,(int) (dr * SAMPLE_RATE));
}

void Echo::out(const Stereo<float *> &input)
{
    REALTYPE ldl, rdl;

    for(int i = 0; i < SOUND_BUFFER_SIZE; ++i) {
        ldl = delay.l()[pos.l()];
        rdl = delay.r()[pos.r()];
        ldl = ldl * (1.0 - lrcross) + rdl * lrcross;
        rdl = rdl * (1.0 - lrcross) + ldl * lrcross;

        efxoutl[i] = ldl * 2.0;
        efxoutr[i] = rdl * 2.0;

        ldl = input.l()[i] * panning - ldl * fb;
        rdl = input.r()[i] * (1.0 - panning) - rdl * fb;

        //LowPass Filter
        old.l() = delay.l()[(pos.l()+delta.l())%(MAX_DELAY * SAMPLE_RATE)] =  ldl * hidamp + old.l() * (1.0 - hidamp);
        old.r() = delay.r()[(pos.r()+delta.r())%(MAX_DELAY * SAMPLE_RATE)] =  rdl * hidamp + old.r() * (1.0 - hidamp);

        //increment
        ++pos.l();// += delta.l();
        ++pos.r();// += delta.r();

        //ensure that pos is still in bounds
        pos.l() %= MAX_DELAY * SAMPLE_RATE;
        pos.r() %= MAX_DELAY * SAMPLE_RATE;

        //adjust delay if needed
        delta.l() = (15*delta.l() + ndelta.l())/16;
        delta.r() = (15*delta.r() + ndelta.r())/16;
    }
}


/*
 * Parameter control
 */
void Echo::setvolume(unsigned char Pvolume)
{
    this->Pvolume = Pvolume;

    if(insertion == 0) {
        outvolume = pow(0.01, (1.0 - Pvolume / 127.0)) * 4.0;
        volume    = 1.0;
    }
    else
        volume = outvolume = Pvolume / 127.0;
    ;
    if(Pvolume == 0)
        cleanup();
}

void Echo::setpanning(unsigned char Ppanning)
{
    this->Ppanning = Ppanning;
    panning = (Ppanning + 0.5) / 127.0;
}

void Echo::setdelay(unsigned char Pdelay)
{
    this->Pdelay=Pdelay;
    avgDelay=(Pdelay/127.0*1.5);//0 .. 1.5 sec
    initdelays();
}

void Echo::setlrdelay(unsigned char Plrdelay)
{
    REALTYPE tmp;
    this->Plrdelay = Plrdelay;
    tmp =
        (pow(2, fabs(Plrdelay - 64.0) / 64.0 * 9) - 1.0) / 1000.0;
    if(Plrdelay < 64.0)
        tmp = -tmp;
    lrdelay = tmp;
    initdelays();
}

void Echo::setlrcross(unsigned char Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = Plrcross / 127.0 * 1.0;
}

void Echo::setfb(unsigned char Pfb)
{
    this->Pfb = Pfb;
    fb = Pfb / 128.0;
}

void Echo::sethidamp(unsigned char Phidamp)
{
    this->Phidamp = Phidamp;
    hidamp = 1.0 - Phidamp / 127.0;
}

void Echo::setpreset(unsigned char npreset)
{
    /**\todo see if the preset array can be replaced with a struct or a class*/
    const int     PRESET_SIZE = 7;
    const int     NUM_PRESETS = 9;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        //Echo 1
        {67, 64, 35,  64,   30,   59,   0  },
        //Echo 2
        {67, 64, 21,  64,   30,   59,   0  },
        //Echo 3
        {67, 75, 60,  64,   30,   59,   10 },
        //Simple Echo
        {67, 60, 44,  64,   30,   0,    0  },
        //Canyon
        {67, 60, 102, 50,   30,   82,   48 },
        //Panning Echo 1
        {67, 64, 44,  17,   0,    82,   24 },
        //Panning Echo 2
        {81, 60, 46,  118,  100,  68,   18 },
        //Panning Echo 3
        {81, 60, 26,  100,  127,  67,   36 },
        //Feedback Echo
        {62, 64, 28,  64,   100,  90,   55 }
    };


    if(npreset >= NUM_PRESETS)
        npreset = NUM_PRESETS - 1;
    for(int n = 0; n < PRESET_SIZE; n++)
        changepar(n, presets[npreset][n]);
    if(insertion)
        setvolume(presets[npreset][0] / 2);         //lower the volume if this is insertion effect
    Ppreset = npreset;
}


void Echo::changepar(int npar, unsigned char value)
{
    switch(npar) {
    case 0:
        setvolume(value);
        break;
    case 1:
        setpanning(value);
        break;
    case 2:
        setdelay(value);
        break;
    case 3:
        setlrdelay(value);
        break;
    case 4:
        setlrcross(value);
        break;
    case 5:
        setfb(value);
        break;
    case 6:
        sethidamp(value);
        break;
    }
}

unsigned char Echo::getpar(int npar) const
{
    switch(npar) {
    case 0:
        return Pvolume;
        break;
    case 1:
        return Ppanning;
        break;
    case 2:
        return Pdelay;
        break;
    case 3:
        return Plrdelay;
        break;
    case 4:
        return Plrcross;
        break;
    case 5:
        return Pfb;
        break;
    case 6:
        return Phidamp;
        break;
    }
    return 0; // in case of bogus parameter number
}

