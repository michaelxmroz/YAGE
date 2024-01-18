#include "AudioWin32.h"
#include <stdexcept>
#include <cmath>

#include <Audioclient.h>
#include <mmdeviceapi.h>

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


float frequencyHz = 440.0;
float phase = 0;
void DebugSine(BYTE* buffer, uint32_t numFrames, unsigned long long sampleRate)
{
    float delta = 2.0f * 3.14159265f * frequencyHz / sampleRate;
    float* fltBuffer = reinterpret_cast<float*>(buffer);
    for (uint32_t i = 0; i < numFrames; ++i)
    {
        float nextSample = std::sin(phase);
        phase += delta;
        fltBuffer[i* 2 + 1] = nextSample;
        fltBuffer[i * 2 ] = 0.0;
    }
}

void AudioWin32::AudioController::PlayAudioStream()
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    WAVEFORMATEXTENSIBLE* wfx = NULL;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE* pData;
    DWORD flags = 0;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

        hr = pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

        if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {

            wfx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
        }

        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            0,
            hnsRequestedDuration,
            0,
            pwfx,
            NULL);
    EXIT_ON_ERROR(hr)

        // Tell the audio source which format to use.
       // hr = pMySource->SetFormat(pwfx);
    //EXIT_ON_ERROR(hr)

        // Get the actual size of the allocated buffer.
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetService(
            IID_IAudioRenderClient,
            (void**)&pRenderClient);
    EXIT_ON_ERROR(hr)

        // Grab the entire buffer for the initial fill operation.
        hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
    EXIT_ON_ERROR(hr)

        // Load the initial data into the shared buffer.
        //hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
    //EXIT_ON_ERROR(hr)
        DebugSine(pData, bufferFrameCount, pwfx->nSamplesPerSec);

        hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
    EXIT_ON_ERROR(hr)

        // Calculate the actual duration of the allocated buffer.
        hnsActualDuration = (double)REFTIMES_PER_SEC *
        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start playing.
    EXIT_ON_ERROR(hr)

        // Each loop fills about half of the shared buffer.
        while (m_state.m_run)
        {
            // Sleep for half the buffer duration.
            Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

            // See how much buffer space is available.
            hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
            EXIT_ON_ERROR(hr)

                numFramesAvailable = bufferFrameCount - numFramesPadding;

            // Grab all the available space in the shared buffer.
            hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
            EXIT_ON_ERROR(hr)

                // Get next 1/2-second of data from the audio source.
               // hr = pMySource->LoadData(numFramesAvailable, pData, &flags);
            //EXIT_ON_ERROR(hr)
                DebugSine(pData, numFramesAvailable, pwfx->nSamplesPerSec);

                hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
            EXIT_ON_ERROR(hr)
        }

    // Wait for last data in buffer to play before stopping.
    //Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

    hr = pAudioClient->Stop();  // Stop playing.
    EXIT_ON_ERROR(hr)
    Sleep((DWORD)(10));
        Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pRenderClient)

        m_state.m_stopped = true;
}

void AudioWin32::Init()
{
    m_audioThread = std::thread(std::ref(m_audio));
}

void AudioWin32::Terminate()
{
    m_audio.m_state.m_run = false;
    while (!m_audio.m_state.m_stopped)
    {
        Sleep((DWORD)(10));
    }

    m_audioThread.join();
}

void AudioWin32::AudioController::operator()()
{
    m_state.m_run = true;
    m_state.m_stopped = false;
    PlayAudioStream();
}