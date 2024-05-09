/*
  ZynAddSubFX - a software synthesizer

  main.c  -  Main file of the synthesizer
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

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

#include <stdio.h> //remove once iostream is used
#include <iostream>

#include <unistd.h>
#include <pthread.h>


#include <windows.h>
#include <winbase.h>

#include "Misc/Master.h"
#include "Misc/Util.h"
#include "Misc/Dump.h"
extern Dump dump;

#if (defined(NONEMIDIIN) || defined(VSTMIDIIN))
#include "Input/NULLMidiIn.h"
#endif

#include "Input/WINMidiIn.h"

#include "UI/MasterUI.h"

#ifdef PAAUDIOOUT
#include "Output/PAaudiooutput.h"
#endif

MasterUI *ui;

using namespace std;

pthread_t thr1;
Master   *master;
int  swaplr    = 0; //1 for left-right swapping
bool usejackit = false;

MidiIn *Midi;
int Pexitprogram = 0; //if the UI set this to 1, the program will exit

/*
 * User Interface thread
 */

void *thread1(void *arg)
{
#ifndef DISABLE_GUI

    ui->showUI();

    while(Pexitprogram == 0) {

    Fl::wait();
    }

#endif //DISABLE_GUI
    return 0;
}

/*
 * Program initialisation
 */

void initprogram()
{
    cerr.precision(1);
    cerr << std::fixed;

    cerr << "Sound Buffer Size = \t" << SOUND_BUFFER_SIZE << " samples" << endl;
    cerr << "Internal latency = \t" << SOUND_BUFFER_SIZE * 1000.0
    / SAMPLE_RATE << " ms" << endl;
    cerr << "ADsynth Oscil.Size = \t" << OSCIL_SIZE << " samples" << endl;

    srand(time(NULL));
    denormalkillbuf = new REALTYPE [SOUND_BUFFER_SIZE];
    for(int i = 0; i < SOUND_BUFFER_SIZE; i++)
        denormalkillbuf[i] = (RND - 0.5) * 1e-16;

    master = new Master();
    master->swaplr = swaplr;

#ifdef PAAUDIOOUT
    PAaudiooutputinit(master);
#endif

#if (defined(NONEMIDIIN) || (defined(VSTMIDIIN)))
    Midi = new NULLMidiIn();
#endif
}

/*
 * Program exit
 */
void exitprogram()
{
    pthread_mutex_lock(&master->mutex);

#ifdef PAAUDIOOUT
    PAfinish();
#endif

#ifndef DISABLE_GUI
    delete (ui);
#endif
    delete (Midi);
    delete (master);

//    pthread_mutex_unlock(&master->mutex);
    delete [] denormalkillbuf;
}

#ifdef OS_WINDOWS
#define ARGSIZE 100
char winoptarguments[ARGSIZE];
char getopt(int argc, char *argv[], const char *shortopts, int *index)
{
    winoptarguments[0] = 0;
    char result = 0;

    if(*index >= argc)
        return -1;

    if(strlen(argv[*index]) == 2)
        if(argv[*index][0] == '-') {
            result = argv[*index][1];
            if(*index + 1 < argc)
                snprintf(winoptarguments, ARGSIZE, "%s", argv[*index + 1]);
            ;
        }
    ;
    (*index)++;
    return result;
}
int opterr = 0;
#undef ARGSIZE

#endif

#ifndef VSTAUDIOOUT
int main(int argc, char *argv[])
{
    config.init();
    dump.startnow();
    int noui = 0;

    cerr
    << "\nZynAddSubFX - Copyright (c) 2002-2009 Nasca Octavian Paul and others"
    << endl;
    cerr << "Compiled: " << __DATE__ << " " << __TIME__ << endl;
    cerr << "This program is free software (GNU GPL v.2 or later) and \n";
    cerr << "it comes with ABSOLUTELY NO WARRANTY.\n" << endl;
#ifdef OS_LINUX
    if(argc == 1)
        cerr << "Try 'zynaddsubfx --help' for command-line options." << endl;
#else
    if(argc == 1)
        cerr << "Try 'zynaddsubfx -h' for command-line options.\n" << endl;
#endif
    /* Get the settings from the Config*/
    SAMPLE_RATE = config.cfg.SampleRate;
    SOUND_BUFFER_SIZE = config.cfg.SoundBufferSize;
    OSCIL_SIZE  = config.cfg.OscilSize;
    swaplr      = config.cfg.SwapStereo;

    /* Parse command-line options */
#ifdef OS_LINUX
    struct option opts[] = {
        {
            "load", 2, NULL, 'l'
        },
        {
            "load-instrument", 2, NULL, 'L'
        },
        {
            "sample-rate", 2, NULL, 'r'
        },
        {
            "buffer-size", 2, NULL, 'b'
        },
        {
            "oscil-size", 2, NULL, 'o'
        },
        {
            "dump", 2, NULL, 'D'
        },
        {
            "swap", 2, NULL, 'S'
        },
        {
            "no-gui", 2, NULL, 'U'
        },
        {
            "not-use-jack", 2, NULL, 'A'
        },
        {
            "dummy", 2, NULL, 'Y'
        },
        {
            "help", 2, NULL, 'h'
        },
        {
            0, 0, 0, 0
        }
    };
#endif
    opterr = 0;
    int option_index = 0, opt, exitwithhelp = 0;

    char loadfile[1001];
    ZERO(loadfile, 1001);
    char loadinstrument[1001];
    ZERO(loadinstrument, 1001);

    while(1) {
        /**\todo check this process for a small memory leak*/
#ifdef OS_LINUX
        opt = getopt_long(argc, argv, "l:L:r:b:o:hSDUAY", opts, &option_index);
        char *optarguments = optarg;
#else
        opt = getopt(argc, argv, "l:L:r:b:o:hSDUAY", &option_index);
        char *optarguments = &winoptarguments[0];
#endif

        if(opt == -1)
            break;

        int tmp;
        switch(opt) {
        case 'h':
            exitwithhelp = 1;
            break;
        case 'Y':/* this command a dummy command (has NO effect)
                and is used because I need for NSIS installer
            (NSIS sometimes forces a command line for a
            program, even if I don't need that; eg. when
            I want to add a icon to a shortcut.
              */
            break;
        case 'U':
            noui = 1;
            break;
        case 'A':
            break;
        case 'l':
            tmp = 0;
            if(optarguments != NULL)
                snprintf(loadfile, 1000, "%s", optarguments);
            ;
            break;
        case 'L':
            tmp = 0;
            if(optarguments != NULL)
                snprintf(loadinstrument, 1000, "%s", optarguments);
            ;
            break;
        case 'r':
            tmp = 0;
            if(optarguments != NULL)
                tmp = atoi(optarguments);
            if(tmp >= 4000)
                SAMPLE_RATE = tmp;
            else {
                cerr << "ERROR:Incorrect sample rate: " << optarguments << endl;
                exit(1);
            }
            break;
        case 'b':
            tmp = 0;
            if(optarguments != NULL)
                tmp = atoi(optarguments);
            if(tmp >= 2)
                SOUND_BUFFER_SIZE = tmp;
            else {
                cerr << "ERROR:Incorrect buffer size: " << optarguments << endl;
                exit(1);
            }
            break;
        case 'o':
            tmp = 0;
            if(optarguments != NULL)
                tmp = atoi(optarguments);
            OSCIL_SIZE = tmp;
            if(OSCIL_SIZE < MAX_AD_HARMONICS * 2)
                OSCIL_SIZE = MAX_AD_HARMONICS * 2;
            OSCIL_SIZE = (int) pow(2, ceil(log(OSCIL_SIZE - 1.0) / log(2.0)));
            if(tmp != OSCIL_SIZE) {
                cerr
                <<
                "\nOSCIL_SIZE is wrong (must be 2^n) or too small. Adjusting to ";
                cerr << OSCIL_SIZE << "." << endl;
            }
            break;
        case 'S':
            swaplr = 1;
            break;
        case 'D':
            dump.startnow();
            break;
        case '?':
            cerr << "ERROR:Bad option or parameter.\n" << endl;
            exitwithhelp = 1;
            break;
        }
    }

    if(exitwithhelp != 0) {
        cout << "Usage: zynaddsubfx [OPTION]\n" << endl;
        cout << "  -h , --help \t\t\t\t display command-line help and exit"
             << endl;
        cout << "  -l file, --load=FILE\t\t\t loads a .xmz file" << endl;
        cout << "  -L file, --load-instrument=FILE\t\t loads a .xiz file"
             << endl;
        cout << "  -r SR, --sample-rate=SR\t\t set the sample rate SR" << endl;
        cout
        << "  -b BS, --buffer-size=SR\t\t set the buffer size (granularity)"
        << endl;
        cout << "  -o OS, --oscil-size=OS\t\t set the ADsynth oscil. size"
             << endl;
        cout << "  -S , --swap\t\t\t\t swap Left <--> Right" << endl;
        cout << "  -D , --dump\t\t\t\t Dumps midi note ON/OFF commands" << endl;
        cout
        << "  -U , --no-gui\t\t\t\t Run ZynAddSubFX without user interface"
        << endl;

#ifdef OS_WINDOWS
        cout
        <<
        "\nWARNING: On Windows systems, only short comandline parameters works."
        << endl;
        cout << "  eg. instead '--buffer-size=512' use '-b 512'" << endl;
#endif
        cout << '\n' << endl;
        return 0;
    }

    initprogram();

    if(strlen(loadfile) > 1) {
        int tmp = master->loadXML(loadfile);
        if(tmp < 0) {
            fprintf(stderr,
                    "ERROR:Could not load master file  %s .\n",
                    loadfile);
            exit(1);
        }
        else {
            master->applyparameters();
            cout << "Master file loaded." << endl;
        }
    }

    if(strlen(loadinstrument) > 1) {
        int loadtopart = 0;
        int tmp = master->part[loadtopart]->loadXMLinstrument(loadinstrument);
        if(tmp < 0) {
            cerr << "ERROR:Could not load instrument file "
                 << loadinstrument << '.' << endl;
            exit(1);
        }
        else {
            master->part[loadtopart]->applyparameters();
            cout << "Instrument file loaded." << endl;
        }
    }

#ifndef DISABLE_GUI
    if(noui == 0) {
        ui = new MasterUI(master, &Pexitprogram);
        pthread_create(&thr1, NULL, thread1, NULL);
    }
#endif

#ifdef WINMIDIIN
    InitWinMidi(master);
#endif

    while(Pexitprogram == 0) {
#ifdef OS_LINUX
        usleep(100000);
#elif OS_WINDOWS
        Sleep(100);
#endif
    }

#ifdef WINMIDIIN
    StopWinMidi();
#endif

    exitprogram();
    return 0;
}

#else

#include "Output/VSTaudiooutput.h"

//define main main_plugin
//extern "C" __declspec(dllexport) AEffect *main_plugin (audioMasterCallback audioMaster);
#define main VSTPluginMain
//AEffect *main (audioMasterCallback audioMaster);
extern "C" __declspec(dllexport) AEffect *main (audioMasterCallback audioMaster);

int instances = -1;
AEffect *main(audioMasterCallback audioMaster)
{
//    if (audioMaster(0,audioMasterVersion,0,0,0,0)!=0) {
//	return(0);
//    };

    if(instances == -1) {
        Midi = new NULLMidiIn();
        denormalkillbuf = new REALTYPE [SOUND_BUFFER_SIZE];
        for(int i = 0; i < SOUND_BUFFER_SIZE; i++)
            denormalkillbuf[i] = (RND - 0.5) * 1e-16;
        instances = 0;
    }

    if(instances != 0)
        return 0;               //don't allow multiple instances

    AudioEffect *sintetizator = new VSTSynth(audioMaster);

    return sintetizator->getAeffect();
}

/*AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
    //AudioEffect *sintetizator = new VSTSynth(audioMaster);
    //return sintetizator->getAeffect();
    return new VSTSynth (audioMaster);
}
*/
void *hInstance;
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
    hInstance = hInst;
    return 1;
}

void *thread(void *arg)
{
    VSTSynth *vs = (VSTSynth *) arg;

    vs->ui = new MasterUI(vs->vmaster, &vs->Pexitprogram);

    vs->ui->showUI();

    while(vs->Pexitprogram == 0)
        Fl::wait(0.01);

    delete (vs->ui);
    Fl::wait(0.01);

    pthread_exit(0);
    return 0;
}

//Parts of the VSTSynth class
VSTSynth::VSTSynth (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1,0)

{
    instances++;

    if(audioMaster) {
        setNumInputs(0);
        setNumOutputs(2);
        setUniqueID('ZASF');
        canProcessReplacing();
//    hasVu(false);
//    hasClip(false);

        isSynth(true);

        programsAreChunks(true);
    }


    SAMPLE_RATE = config.cfg.SampleRate;
    SOUND_BUFFER_SIZE = config.cfg.SoundBufferSize;
    OSCIL_SIZE  = config.cfg.OscilSize;
    swaplr      = config.cfg.SwapStereo;
    this->Pexitprogram    = 0;

    this->vmaster         = new Master();
    this->vmaster->swaplr = swaplr;


//    FILE *a=fopen("aaaa0","a");
//    fprintf(a,"%lx %lx %lx\n",this,this->ui,this->ui->masterwindow);
//    fflush(a);fclose(a);

    pthread_create(&this->thr, NULL, thread, this);

//    suspend();
}

VSTSynth::~VSTSynth()
{
    this->Pexitprogram = 1;

    Sleep(200); //wait the thread to finish

//    pthread_mutex_lock(&vmaster->mutex);


    delete (this->vmaster);

    instances--;
}

int VSTSynth::processEvents(VstEvents *events)
{
    for(int i = 0; i < events->numEvents; i++) {
        //debug stuff
//      FILE *a=fopen("events","a");
//      fprintf(a,"%lx\n",events->events[i]->type);
//      fflush(a);fclose(a);

        if((events->events[i])->type != kVstMidiType)
            continue;
        VstMidiEvent  *ev   = (VstMidiEvent *) events->events[i];
        unsigned char *data = (unsigned char *)ev->midiData;
        int status  = data[0] / 16;
        int cmdchan = data[0] & 0x0f;
        int cntl;

        pthread_mutex_lock(&vmaster->mutex);
        switch(status) {
        case 0x8:
            vmaster->NoteOff(cmdchan, data[1] & 0x7f);
            break;
        case 0x9:
            if(data[2] == 0)
                vmaster->NoteOff(cmdchan, data[1] & 0x7f);
            else
                vmaster->NoteOn(cmdchan, data[1] & 0x7f, data[2] & 0x7f);
            break;
        case 0xB:
            cntl = Midi->getcontroller(data[1] & 0x7f);
            vmaster->SetController(cmdchan, cntl, data[2] & 0x7f);
            break;
        case 0xE:
            vmaster->SetController(cmdchan, C_pitchwheel, data[1] + data[2]
                                   * (long int) 128 - 8192);
            break;
        }
        pthread_mutex_unlock(&vmaster->mutex);
    }

    return 1;
}

int VSTSynth::getChunk(void **data, bool isPreset)
{
    int size = 0;
    size = vmaster->getalldata((char **)data);
    return (int)size;
}

long VSTSynth::setChunk(void *data, long size, bool isPreset)
{
    vmaster->putalldata((char *)data, size);
    return 0;
}
#endif

