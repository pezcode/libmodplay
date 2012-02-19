#ifndef MOD_PLAY
#define MOD_PLAY

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include "modplug.h"

class ModPlay
{
public:

	ModPlay(const void* data, size_t size);
	~ModPlay();

	void  play();
	void pause();

private:

	ModPlay(const ModPlay&);
	ModPlay& operator=(const ModPlay&);

	static const unsigned int CHANNELS = 2;
	static const unsigned int FREQUENCY = 44100;
	static const unsigned int BITS = 16;
	static const unsigned int BLOCKSIZE = (BITS/8)*CHANNELS;
	static const unsigned int BYTESPERSEC = BLOCKSIZE * FREQUENCY;
	static const size_t BUFSIZE = BYTESPERSEC/10; // ~100ms
	static const size_t BUFFERS = 2;

	void initPlayer();
	bool loadData(const void* data, size_t size);
	void fillBuffer(size_t buffer);

	bool initDevice();
	bool startListener();

	//static const char EVENT_DONE_NAME[];
	HANDLE hEventDone;
	HANDLE hThread;
	ModPlugFile* modFile;
	HWAVEOUT hWaveOut;
	bool stopped;
	BYTE buffers[BUFFERS][BUFSIZE];
	WAVEHDR headers[BUFFERS];
	size_t currentBuffer;

	static void CALLBACK waveOutCallback(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
};

#endif
