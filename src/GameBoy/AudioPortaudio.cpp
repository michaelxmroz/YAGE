#include "AudioPortaudio.h"
#include <stdio.h>
#include <iostream>

#define NUM_SECONDS 4
#define SAMPLE_RATE 48000
#define BUFFER_SIZE_SECONDS 1
#define PLAYBACK_OFFSET_MS 0

#define AUDIO_DEBUG 1

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/

void AudioPortaudio::Init()
{
    m_buffer = new float[GetAudioBufferSize()]();
    m_writePosition = static_cast<uint32_t>((static_cast<float>(SAMPLE_RATE) / 1000.0f) * PLAYBACK_OFFSET_MS);
    m_playbackPosition = 0;


    PaError err;

    err = Pa_Initialize();
    if (err != paNoError)
    {
        ErrorHandler(err);
        return;
    }

    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream(&m_stream,
        0,          /* no input channels */
        2,          /* stereo output */
        paFloat32,  /* 32 bit floating point output */
        SAMPLE_RATE,
        256,        /* frames per buffer */
        paCallback,
        this);
    if (err != paNoError)
    {
        ErrorHandler(err);
        return;
    }
}



void AudioPortaudio::Terminate()
{
    PaError err;
    err = Pa_StopStream(m_stream);
    if (err != paNoError)
    {
        ErrorHandler(err);
        return;
    }
    err = Pa_CloseStream(m_stream);
    if (err != paNoError)
    {
        ErrorHandler(err);
        return;
    }
    Pa_Terminate();
    delete[] m_buffer;
}

void AudioPortaudio::Play()
{
    m_framesConsumed = 0;
    PaError err = Pa_StartStream(m_stream);
    if (err != paNoError)
    {
        ErrorHandler(err);
    }
}

float* AudioPortaudio::GetAudioBuffer()
{
    return m_buffer;
}

uint32_t AudioPortaudio::GetAudioBufferSize()
{
    return SAMPLE_RATE * BUFFER_SIZE_SECONDS * 2;
}

uint32_t AudioPortaudio::GetSampleRate()
{
    return SAMPLE_RATE;
}

uint32_t* AudioPortaudio::GetWritePosition()
{
    return &m_writePosition;
}

uint32_t AudioPortaudio::GetFramesConsumed()
{
    return m_framesConsumed;
}

void AudioPortaudio::ResetFramesConsumed()
{
    m_framesConsumed = 0;
}

void AudioPortaudio::ErrorHandler(PaError err)
{
    //TODO use logger
    Pa_Terminate();
    fprintf(stderr, "An error occurred while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
}

inline int AudioPortaudio::paCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    /* Cast data passed through stream to our structure. */
    AudioPortaudio* data = (AudioPortaudio*)userData;
    float* out = (float*)outputBuffer;
    unsigned int i;
    (void)inputBuffer; /* Prevent unused variable warning. */

    for (i = 0; i < framesPerBuffer; i++)
    {
        data->m_framesConsumed++;

        if (data->m_playbackPosition == data->m_writePosition)
        {
           // fprintf(stderr, "Playback running ahead of audio generation\n");
            *out++ = 0;
            *out++ = 0;
            continue;
        }
        float* left = data->m_buffer + data->m_playbackPosition++;
        float* right = data->m_buffer + data->m_playbackPosition++;

        *out++ = *left;
        *out++ = *right;

#if AUDIO_DEBUG
       
        *left = 0;
        *right = 0;

        if (data->m_playbackPosition == data->m_writePosition + 2)
        {
            fprintf(stderr, "Playback running ahead of audio generation\n");
        }
        uint32_t buffSize = data->GetAudioBufferSize();
        //std::cout << "Dist: " << (data->m_writePosition + buffSize - data->m_playbackPosition) % buffSize<< std::endl;

#endif
        if (data->m_playbackPosition >= data->GetAudioBufferSize())
        {
            data->m_playbackPosition = 0;
        }
    }
    return 0;
}
