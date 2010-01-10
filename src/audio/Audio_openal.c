

/*
 * Authors: Heikki Aitakangas, Finland 2009
 *          Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alut.h>

#include <Audio.h>
#include <Audio_openal.h>

#include <kunquat/Player.h>

#include <xmemory.h>


#define FREQUENCY 44100
#define NUM_BUFS 3
#define BUF_LENGTH_MS 100
#define NUM_FRAMES ((FREQUENCY * BUF_LENGTH_MS) / 1000)


struct Audio_openal
{
    Audio     parent;

    bool      alut_inited;
    ALuint    source;
    ALuint    al_bufs[NUM_BUFS];

    bool      thread_active;
    pthread_t play_thread;
    
    int16_t*  out_buf;
};


static void Audio_openal_mix_buffer(Audio_openal* audio_openal, ALuint buffer);

static void* Audio_openal_thread(void* data);

static bool Audio_openal_set_buffer_size(Audio_openal* audio_openal, uint32_t nframes);

static bool Audio_openal_set_freq(Audio_openal* audio_openal, uint32_t freq);

static bool Audio_openal_open(Audio_openal* audio_openal);

static bool Audio_openal_close(Audio_openal* audio_openal);

static void del_Audio_openal(Audio_openal* audio_openal);


#define close_if_false(EXPR,MSG)                                  \
    if (true) {                                                   \
        if (!(EXPR))                                              \
        {                                                         \
            Audio_set_error(audio, "OpenAL driver: %s\n", (MSG)); \
            Audio_openal_close(audio_openal);                     \
            return false;                                         \
        }                                                         \
    } else (void)0

#define close_if_al_error(STMT,MSG)                               \
    if (true) {                                                   \
        (STMT);                                                   \
        if (alGetError() != AL_NO_ERROR)                          \
        {                                                         \
            Audio_set_error(audio, "OpenAL driver: %s\n", (MSG)); \
            Audio_openal_close(audio_openal);                     \
            return false;                                         \
        }                                                         \
    } else (void)0

#define end_if_al_error(STMT,MSG)                                         \
    if (true) {                                                           \
        (STMT);                                                           \
        if (alGetError() != AL_NO_ERROR)                                  \
        {                                                                 \
            Audio_set_error(audio, "OpenAL driver: thread: %s\n", (MSG)); \
            audio_openal->parent.active = false;                          \
            return NULL;                                                  \
        }                                                                 \
    } else (void)0


Audio* new_Audio_openal(void)
{
    Audio_openal* audio_openal = xalloc(Audio_openal);
    if (audio_openal == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_openal->parent,
                    "openal",
                    (bool (*)(Audio*))Audio_openal_open,
                    (bool (*)(Audio*))Audio_openal_close,
                    (void (*)(Audio*))del_Audio_openal))
    {
        xfree(audio_openal);
        return NULL;
    }
    audio_openal->parent.set_buffer_size =
            (bool (*)(Audio*, uint32_t))Audio_openal_set_buffer_size;
    audio_openal->parent.set_freq =
            (bool (*)(Audio*, uint32_t))Audio_openal_set_freq;

    // Reserving work buffers
    audio_openal->out_buf = xcalloc(int16_t, NUM_FRAMES * 2); // Stereo
    if (audio_openal->out_buf == NULL)
    {
        xfree(audio_openal);
        return NULL;
    }
    
    // Initial state is all empty values
    // Can't use 0 for the OpenAL source & buffer values, since that's
    // a special, always valid, value for them. Hopefully -1 isn't.
    audio_openal->alut_inited = false;
    audio_openal->source = (ALuint)-1;
    for (int i = 0; i < NUM_BUFS; ++i)
    {
        audio_openal->al_bufs[i] = (ALuint)-1;
    }
    audio_openal->parent.active = audio_openal->thread_active = false;
    audio_openal->parent.nframes = NUM_FRAMES;
    audio_openal->parent.freq = FREQUENCY;

    return &audio_openal->parent;
}


static bool Audio_openal_set_buffer_size(Audio_openal* audio_openal, uint32_t nframes)
{
    assert(audio_openal != NULL);
    assert(nframes > 0);
    Audio* audio = &audio_openal->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set buffer size while the driver is active.");
        return false;
    }
    if (audio->handle != NULL)
    {
        if (kqt_Handle_set_buffer_size(audio->handle, nframes))
        {
            Audio_set_error(audio, kqt_Handle_get_error(audio->handle));
            audio->handle = NULL;
            return false;
        }
    }
    int16_t* new_buf = xrealloc(int16_t, nframes * 2, audio_openal->out_buf);
    if (new_buf == NULL)
    {
        Audio_set_error(audio, "Couldn't allocate memory for new output buffers.");
        return false;
    }
    audio_openal->out_buf = new_buf;
    audio->nframes = nframes;
    return true;
}


static bool Audio_openal_set_freq(Audio_openal* audio_openal, uint32_t freq)
{
    assert(audio_openal != NULL);
    assert(freq > 0);
    Audio* audio = &audio_openal->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set mixing frequency while the driver is active.");
        return false;
    }
    audio->freq = freq;
    return true;
}


static bool Audio_openal_open(Audio_openal* audio_openal)
{
    assert(audio_openal != NULL);
    Audio* audio = &audio_openal->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Driver is already active");
        return false;
    }
    
    // Using alut here, since there's no need - for now - to use
    // other than the default audio device.
    if (alutInit(NULL, NULL) != AL_TRUE)
    {
        const char* err_str = alutGetErrorString(alutGetError());
        Audio_set_error(audio, "OpenAL initialization failed: %s\n", err_str);
        return false;
    }
    audio_openal->alut_inited = true;
    
    // Making sure the OpenAL error status is not set, operations later on
    // will be checking it
    alGetError();
    
    // Reserving OpenAL resources
    close_if_al_error(alGenSources(1, &audio_openal->source),
                      "Couldn't generate source.");
    close_if_al_error(alGenBuffers(NUM_BUFS, audio_openal->al_bufs),
                      "Couldn't generate buffers.");
    
    // These source properties are necessary when playing mono sound or when
    // the OpenAL system is used simultaneously for some other playback
    // besides Kunquat
    alSource3f(audio_openal->source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(audio_openal->source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(audio_openal->source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef( audio_openal->source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei( audio_openal->source, AL_SOURCE_RELATIVE, AL_TRUE      );
    
    // Mix first bufferfulls and queue them
    for (int i = 0; i < NUM_BUFS; ++i)
    {
        close_if_al_error(Audio_openal_mix_buffer(audio_openal, audio_openal->al_bufs[i]),
                          "Failed to buffer data.");
    }
    close_if_al_error(alSourceQueueBuffers(audio_openal->source, NUM_BUFS, audio_openal->al_bufs),
                      "Failed to queue inital buffers.");
    
    // Start the processing thread. It will set the audio source playing
    audio->active = true;
    close_if_false(!pthread_create(&audio_openal->play_thread, NULL,
                                   Audio_openal_thread, audio_openal),
                   "Thread creation failed.");
    audio_openal->thread_active = true;
    return true;
}


static void Audio_openal_mix_buffer(Audio_openal* audio_openal, ALuint buffer)
{
    assert(audio_openal != NULL);
    assert(audio_openal->out_buf != NULL);
    Audio* audio = &audio_openal->parent;
    
    // Generate the sound
    uint32_t mixed = 0;
    kqt_Handle* handle = audio->handle;
    if (handle != NULL && !audio->pause)
    {
        mixed = kqt_Handle_mix(handle, audio->nframes, audio->freq);
        float* bufs[KQT_BUFFERS_MAX] = {
            kqt_Handle_get_buffer(handle, 0),
            kqt_Handle_get_buffer(handle, 1)
        };
        // Convert to interleaved 16-bit stereo
        for (uint32_t i = 0; i < audio->nframes; ++i)
        {
            audio_openal->out_buf[i * 2] = (int16_t)(bufs[0][i] * INT16_MAX);
            audio_openal->out_buf[(i * 2) + 1] = (int16_t)(bufs[1][i] * INT16_MAX);
        }
    }
    for (uint32_t i = mixed * 2; i < audio->nframes * 2; ++i)
    {
        audio_openal->out_buf[i] = 0;
    }
    Audio_notify(audio);
    
    // Have OpenAL buffer the data. It will be copied from out_buf
    // to internal data structures.
    // Checking & handling errors from this are the responsibility
    // of the caller.
    alBufferData(buffer, AL_FORMAT_STEREO16, audio_openal->out_buf,
                 sizeof(int16_t) * 2 * audio->nframes, audio->freq);

    return;
}


static void* Audio_openal_thread(void* data)
{
    assert(data != NULL);
    Audio_openal* audio_openal = data;
    Audio* audio = &audio_openal->parent;
    
    // Poll the OpenAL system every 0.5 buffer lengths to see if it's finished
    // processing any yet. If any are finished processing, unqueue, refill and
    // requeue them.
    // If the source had run out of data to play - or hadn't been started yet -
    // set it playing.
    while (audio->active)
    {
        ALint processed;
        ALenum state;

        end_if_al_error(alGetSourcei(audio_openal->source, AL_BUFFERS_PROCESSED, &processed),
                        "Couldn't get number of processed buffers.");        
        while (processed > 0)
        {
            ALuint buffer;
            
            end_if_al_error(alSourceUnqueueBuffers(audio_openal->source, 1, &buffer),
                            "Couldn't unqueue a buffer.");
            end_if_al_error(Audio_openal_mix_buffer(audio_openal, buffer),
                            "Failed to buffer data.");
            end_if_al_error(alSourceQueueBuffers(audio_openal->source, 1, &buffer),
                            "Failed to queue buffer.");
            --processed;
        }
        
        end_if_al_error(alGetSourcei(audio_openal->source, AL_SOURCE_STATE, &state),
                        "Couldn't get source state.");
        if (state != AL_PLAYING)
        {
            end_if_al_error(alSourcePlay(audio_openal->source),
                            "Couldn't start source playing");
        }
        
        alutSleep((0.5 * BUF_LENGTH_MS) / 1000);
    }
    
    return NULL;
}


static bool Audio_openal_close(Audio_openal* audio_openal)
{
    assert(audio_openal != NULL);
    audio_openal->parent.active = false;
    if (audio_openal->thread_active)
    {
        pthread_join(audio_openal->play_thread, NULL);
        audio_openal->thread_active = false;
    }
    if (audio_openal->alut_inited)
    {
        if (alIsSource(audio_openal->source))
        {
            alDeleteSources(1, &audio_openal->source);
            audio_openal->source = (ALuint)-1;
        }
        for (int i = 0; i < NUM_BUFS; ++i)
        {
            if (alIsBuffer(audio_openal->al_bufs[i]))
            {
                alDeleteBuffers(1, &audio_openal->al_bufs[i]);
                audio_openal->al_bufs[i] = (ALuint)-1;
            }
        }
        alutExit();
        audio_openal->alut_inited = false;
    }
    return true;
}


static void del_Audio_openal(Audio_openal* audio_openal)
{
    assert(audio_openal != NULL);
    assert(!audio_openal->parent.active);
    if (audio_openal->out_buf != NULL)
    {
        xfree(audio_openal->out_buf);
        audio_openal->out_buf = NULL;
    }
    return;
}


