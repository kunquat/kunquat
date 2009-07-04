

/*
 * Copyright 2009 Heikki Aitakangas, Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
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

static bool Audio_openal_open(Audio_openal* audio_openal);

static bool Audio_openal_close(Audio_openal* audio_openal);

static void del_Audio_openal(Audio_openal* audio_openal);


#define close_if_false(EXPR,MSG)                                  \
    do {                                                          \
        if (!(EXPR))                                              \
        {                                                         \
            fprintf(stderr, "OpenAL driver: %s\n", (MSG));        \
            Audio_openal_close(audio_openal);                     \
            return false;                                         \
        }                                                         \
    } while(false)

#define close_if_al_error(STMT,MSG)                           \
    do {                                                      \
        (STMT);                                               \
        if (alGetError() != AL_NO_ERROR)                      \
        {                                                     \
            fprintf(stderr, "OpenAL driver: %s\n", (MSG));    \
            Audio_openal_close(audio_openal);                 \
            return false;                                     \
        }                                                     \
    } while(false)

#define end_if_al_error(STMT,MSG)                                       \
    do {                                                                \
        (STMT);                                                         \
        if (alGetError() != AL_NO_ERROR)                                \
        {                                                               \
            fprintf(stderr, "OpenAL driver: thread: %s\n", (MSG));      \
            audio_openal->parent.active = false;                        \
            return NULL;                                                \
        }                                                               \
    } while(false)


Audio* new_Audio_openal(void)
{
    Audio_openal* audio_openal = xalloc(Audio_openal);
    if (audio_openal == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_openal->parent,
                    (bool (*)(Audio*))Audio_openal_open,
                    (bool (*)(Audio*))Audio_openal_close,
                    (void (*)(Audio*))del_Audio_openal))
    {
        xfree(audio_openal);
        return NULL;
    }

    // Reserving work buffers
    audio_openal->out_buf = xcalloc(int16_t, NUM_FRAMES * 2); // Stereo
    if (audio_openal->out_buf == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for the audio buffer.");
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

    return &audio_openal->parent;
}


static bool Audio_openal_open(Audio_openal* audio_openal)
{
    assert(audio_openal != NULL);
    if (audio_openal->parent.active)
    {
        return false;
    }
    
    // Using alut here, since there's no need - for now - to use
    // other than the default audio device.
    if (alutInit(NULL, NULL) != AL_TRUE)
    {
        const char* err_str = alutGetErrorString(alutGetError());
        fprintf(stderr, "OpenAL initialization failed: %s\n", err_str);
        return false;
    }
    audio_openal->alut_inited = true;
    audio_openal->parent.nframes = NUM_FRAMES;
    audio_openal->parent.freq = FREQUENCY;
    
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
    audio_openal->parent.active = true;
    close_if_false(!pthread_create(&audio_openal->play_thread, NULL,
                                   Audio_openal_thread, audio_openal),
                   "Thread creation failed.");
    audio_openal->thread_active = true;
    return true;
}


static void Audio_openal_mix_buffer(Audio_openal* audio_openal, ALuint buffer)
{
    assert(audio_openal->out_buf != NULL);
    
    // Generate the sound
    kqt_Context* context = audio_openal->parent.context;
    if (context != NULL)
    {
        /*uint32_t mixed =*/ kqt_Context_mix(context, NUM_FRAMES, // nframes??
                                             audio_openal->parent.freq);
        int buf_count = kqt_Context_get_buffer_count(context);
        kqt_frame** bufs = kqt_Context_get_buffers(context);
        
        // Convert to interleaved 16-bit stereo
        for (int i = 0; i < NUM_FRAMES; ++i)
        {
            audio_openal->out_buf[i * 2] = (int16_t)(bufs[0][i] * INT16_MAX);
            if (buf_count > 1)
            {
                audio_openal->out_buf[(i * 2) + 1] = (int16_t)(bufs[1][i] * INT16_MAX);
            }
            else
            {
                audio_openal->out_buf[(i * 2) + 1] = audio_openal->out_buf[i * 2];
            }
        }
    }
    Audio_notify(&audio_openal->parent);
    
    // Have OpenAL buffer the data. It will be copied from out_buf
    // to internal data structures.
    // Checking & handling errors from this are the responsibility
    // of the caller.
    alBufferData(buffer, AL_FORMAT_STEREO16, audio_openal->out_buf,
                 sizeof(int16_t) * 2 * NUM_FRAMES, FREQUENCY);

    return;
}


static void* Audio_openal_thread(void* data)
{
    assert(data != NULL);
    Audio_openal* audio_openal = data;
    
    // Poll the OpenAL system every 0.5 buffer lengths to see if it's finished
    // processing any yet. If any are finished processing, unqueue, refill and
    // requeue them.
    // If the source had run out of data to play - or hadn't been started yet -
    // set it playing.
    while (audio_openal->parent.active)
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


