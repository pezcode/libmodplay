#include "modplay.h"

//const char ModPlay::EVENT_DONE_NAME[] = "MODPLAY_EVENT_DONE";

ModPlay::ModPlay(const void* data, size_t size) : hWaveOut(NULL), hThread(NULL), hEventDone(NULL), currentBuffer(0), stopped(false)
{
	initPlayer();
	if(loadData(data, size))
	{
		if(startListener() && initDevice())
		{
			for(int i = 0; i < BUFFERS; i++)
			{
				memset(&headers[i], 0, sizeof(WAVEHDR));
				headers[i].lpData = reinterpret_cast<LPSTR>(buffers[i]);
				headers[i].dwBufferLength = BUFSIZE;
				fillBuffer(i);
			}
		}
	}
}

ModPlay::~ModPlay()
{
	stopped = true; // tell the thread to exit
	waveOutPause(hWaveOut); // make sure no buffers are written and played in case of a race condition
	waveOutReset(hWaveOut); // calls WOM_DONE for all buffers (thread checks if stopped == true and exits)
	WaitForSingleObject(hThread, INFINITE); // Let the thread finish
	waveOutClose(hWaveOut);
	CloseHandle(hEventDone);
	CloseHandle(hThread);
	ModPlug_Unload(modFile);
}

bool ModPlay::loadData(const void* data, size_t size)
{
	modFile = ModPlug_Load(data, size);
	return (modFile != 0);
}

void ModPlay::play()
{
	waveOutRestart(hWaveOut);
}

void ModPlay::pause()
{
	waveOutPause(hWaveOut);
}

void ModPlay::initPlayer() // Set up libmodplug
{
	ModPlug_Settings settings;
	ModPlug_GetSettings(&settings);
	settings.mFrequency = FREQUENCY;
	settings.mBits = BITS;
	settings.mChannels = CHANNELS;
	settings.mLoopCount = -1; // endless
	ModPlug_SetSettings(&settings);
}

bool ModPlay::initDevice() // Set up WINMM
{
	WAVEFORMATEX wfx = {0};
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = FREQUENCY;
	wfx.wBitsPerSample = BITS;
	wfx.nChannels = CHANNELS;
	wfx.nBlockAlign = BLOCKSIZE;
	wfx.nAvgBytesPerSec = BYTESPERSEC;
	if(MMSYSERR_NOERROR == waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, reinterpret_cast<DWORD_PTR>(&waveOutCallback), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION))
	{
		waveOutPause(hWaveOut);
		return true;
	}
	hWaveOut = NULL;
	return false;
}

bool ModPlay::startListener()
{
	hEventDone = CreateEvent(NULL, false, false, NULL/*EVENT_DONE_NAME*/);
	if(hEventDone/* && GetLastError() != ERROR_ALREADY_EXISTS*/)
	{
		DWORD threadID;
		hThread = CreateThread(NULL, 0, &ThreadProc, static_cast<LPVOID>(this), 0, &threadID);
		if(hThread)
		{
			return true;
		}
		CloseHandle(hEventDone);
	}
	hEventDone = NULL;
	return false;
}

void ModPlay::fillBuffer(size_t buffer)
{
	WAVEHDR* hdr = &headers[buffer];
	waveOutUnprepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));

	int total = 0;
	while(total != BUFSIZE)
	{
		int read = ModPlug_Read(modFile, hdr->lpData, BUFSIZE - total);
		if(read == 0)
		{
			ModPlug_Seek(modFile, 0);
		}
		total += read;
	}

	waveOutPrepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, hdr, sizeof(WAVEHDR));
}

void ModPlay::waveOutCallback(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	switch(uMsg)
	{
	case WOM_DONE:
		ModPlay* instance = reinterpret_cast<ModPlay*>(dwInstance);
		SetEvent(instance->hEventDone);
		/*
		HANDLE hEvent;
		hEvent = OpenEvent(EVENT_MODIFY_STATE, false, EVENT_DONE_NAME);
		SetEvent(hEvent);
		CloseHandle(hEvent);
		*/
		break;
	}
}

DWORD ModPlay::ThreadProc(LPVOID lpParameter) // Refills played buffers
{
	ModPlay* instance = reinterpret_cast<ModPlay*>(lpParameter);
	while(WAIT_OBJECT_0 == WaitForSingleObject(instance->hEventDone, INFINITE) && !instance->stopped)
	{
		instance->fillBuffer(instance->currentBuffer);
		instance->currentBuffer = (instance->currentBuffer + 1) % BUFFERS;
	}
	return 0;
}
