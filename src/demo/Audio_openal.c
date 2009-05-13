

/*
 * Copyright 2009 Heikki Aitakangas
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

#include <kunquat.h>

#include "Audio_openal.h"


#define FREQUENCY 44100
#define NUM_BUFS 3
#define BUF_LENGTH_MS 100
#define NUM_FRAMES ((FREQUENCY * BUF_LENGTH_MS) / 1000)


typedef struct Audio_openal
{
    bool      alut_inited;
    ALuint    source;
    ALuint    al_bufs[NUM_BUFS];

    bool      active;
    bool      thread_active;
    pthread_t play_thread;

    Playlist* pl;
    
    int16_t*  out_buf;
    frame_t*  mix_bufs[2];
} Audio_openal;


static Audio_openal context_;
static Audio_openal* context = &context_;


static void Audio_openal_mix_buffer(Audio_openal* context, ALuint buffer);
static void* Audio_openal_thread(void* data);


#define close_if_false(EXPR,MSG)                                  \
    do {                                                          \
        if (!(EXPR))                                              \
        {                                                         \
            fprintf(stderr, "OpenAL driver: %s\n", (MSG));        \
            Audio_openal_close();                                 \
            return false;                                         \
        }                                                         \
    } while(false)

#define close_if_al_error(STMT,MSG)                           \
    do {                                                      \
        STMT;                                                 \
        if (alGetError() != AL_NO_ERROR)                      \
        {                                                     \
            fprintf(stderr, "OpenAL driver: %s\n", (MSG));    \
            Audio_openal_close();                             \
            return false;                                     \
        }                                                     \
    } while(false)

#define end_if_al_error(STMT,MSG)                                       \
    do {                                                                \
        STMT;                                                           \
        if (alGetError() != AL_NO_ERROR)                                \
        {                                                               \
            fprintf(stderr, "OpenAL driver: thread: %s\n", (MSG));      \
            context->active = false;                                    \
            return NULL;                                                \
        }                                                               \
    } while(false)


bool Audio_openal_open(Playlist* pl)
{
    assert(pl != NULL);
    
    // Initial state is all empty values
    // Can't use 0 for the OpenAL source & buffer values, since that's
    // a special, always valid, value for them. Hopefully -1 isn't.
    context->alut_inited = false;
    context->source = (ALuint)-1;
    for (int i = 0; i < NUM_BUFS; ++i)
    {
        context->al_bufs[i] = (ALuint)-1;
    }
    context->active = context->thread_active = false;
    context->pl = pl;
    context->out_buf = NULL;
    context->mix_bufs[0] = context->mix_bufs[1] = NULL;
    
    // Using alut here, since there's no need - for now - to use
    // other than the default audio device.
    if (alutInit(NULL, NULL) != AL_TRUE)
    {
        const char* err_str = alutGetErrorString(alutGetError());
        fprintf(stderr, "OpenAL initialization failed: %s\n", err_str);
        return false;
    }
    context->alut_inited = true;
    
    // Making sure the OpenAL error status is not set, operations later on
    // will be checking it
    alGetError();
    
    // Reserving OpenAL resources
    close_if_al_error(alGenSources(1, &context->source),
                      "Couldn't generate source.");
    close_if_al_error(alGenBuffers(NUM_BUFS, context->al_bufs),
                      "Couldn't generate buffers.");
    
    // These source properties are necessary when playing mono sound or when
    // the OpenAL system is used simultaneously for some other playback
    // besides Kunquat
    alSource3f(context->source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(context->source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(context->source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef( context->source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei( context->source, AL_SOURCE_RELATIVE, AL_TRUE      );
    
    // Reserving work buffers
    context->out_buf = calloc(NUM_FRAMES * 2, sizeof(int16_t)); // Stereo
    close_if_false(context->out_buf != NULL,
                   "Couldn't allocate memory for the audio buffer.");
    for (int i = 0; i < 2; ++i)
    {
        context->mix_bufs[i] = calloc(NUM_FRAMES, sizeof(frame_t));
        close_if_false(context->mix_bufs[i] != NULL,
                       "Couldn't allocate memory for the mixing buffer.");
    }
    
    // Playlist setup
    close_if_false(Playlist_set_buf_size(context->pl, NUM_FRAMES),
                   "Couldn't allocate memory for mixing buffers.");
    Playlist_set_mix_freq(context->pl, FREQUENCY);
    
    // Mix first bufferfulls and queue them
    for (int i = 0; i < NUM_BUFS; ++i)
    {
        close_if_al_error(Audio_openal_mix_buffer(context, context->al_bufs[i]),
                          "Failed to buffer data.");
    }
    close_if_al_error(alSourceQueueBuffers(context->source, NUM_BUFS, context->al_bufs),
                      "Failed to queue inital buffers.");
    
    // Start the processing thread. It will set the audio source playing
    context->active = true;
    close_if_false(!pthread_create(&context->play_thread, NULL,
                                   Audio_openal_thread, context),
                   "Thread creation failed.");
    context->thread_active = true;
    
    return true;
}

static void Audio_openal_mix_buffer(Audio_openal* context, ALuint buffer)
{
    assert(context->pl          != NULL);
    assert(context->out_buf     != NULL);
    assert(context->mix_bufs[0] != NULL);
    assert(context->mix_bufs[1] != NULL);
    
    // Clear mixing buffers, Playlist_mix doesn't do it for us
    memset(context->mix_bufs[0], 0, sizeof(frame_t) * NUM_FRAMES);
    memset(context->mix_bufs[1], 0, sizeof(frame_t) * NUM_FRAMES);
    
    // Generate the sound
    Playlist_mix(context->pl, NUM_FRAMES, context->mix_bufs);
    
    // Convert to interleaved 16-bit stereo
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        context->out_buf[i * 2]       = (int16_t)(context->mix_bufs[0][i] * INT16_MAX);
        context->out_buf[(i * 2) + 1] = (int16_t)(context->mix_bufs[1][i] * INT16_MAX);
        
    }
    
    // Have OpenAL buffer the data. It will be copied from out_buf
    // to internal data structures.
    // Checking & handling errors from this are the responsibility
    // of the caller.
    alBufferData(buffer, AL_FORMAT_STEREO16, context->out_buf,
                 sizeof(int16_t) * 2 * NUM_FRAMES, FREQUENCY);
}

static void* Audio_openal_thread(void* data)
{
    assert(data != NULL);
    Audio_openal* context = data;
    
    // Poll the OpenAL system every 0.5 buffer lengths to see if it's finished
    // processing any yet. If any are finished processing, unqueue, refill and
    // requeue them.
    // If the source had run out of data to play - or hadn't been started yet -
    // set it playing.
    while (context->active)
    {
        ALint processed;
        ALenum state;

        end_if_al_error(alGetSourcei(context->source, AL_BUFFERS_PROCESSED, &processed),
                        "Couldn't get number of processed buffers.");        
        while(processed > 0)
        {
            ALuint buffer;
            
            end_if_al_error(alSourceUnqueueBuffers(context->source, 1, &buffer),
                            "Couldn't unqueue a buffer.");
            end_if_al_error(Audio_openal_mix_buffer(context, buffer),
                            "Failed to buffer data.");
            end_if_al_error(alSourceQueueBuffers(context->source, 1, &buffer),
                            "Failed to queue buffer.");
            --processed;
        }
        
        end_if_al_error(alGetSourcei(context->source, AL_SOURCE_STATE, &state),
                        "Couldn't get source state.");
        if (state != AL_PLAYING)
        {
            end_if_al_error(alSourcePlay(context->source),
                            "Couldn't start source playing");
        }
        
        alutSleep((0.5 * BUF_LENGTH_MS) / 1000);
    }
    
    return NULL;
}

void Audio_openal_close(void)
{
    context->active = false;
    if (context->thread_active)
    {
        pthread_join(context->play_thread, NULL);
        context->thread_active = false;
    }
    if (context->alut_inited)
    {
        if (alIsSource(context->source))
        {
            alDeleteSources(1, &context->source);
            context->source = (ALuint)-1;
        }
        for (int i = 0; i < NUM_BUFS; ++i)
        {
            if (alIsBuffer(context->al_bufs[i]))
            {
                alDeleteBuffers(1, &context->al_bufs[i]);
                context->al_bufs[i] = (ALuint)-1;
            }
        }
        alutExit();
        context->alut_inited = false;
    }
    
    context->pl = NULL;
    
    if (context->out_buf != NULL)
    {
        free(context->out_buf);
        context->out_buf = NULL;
    }
    for (int i = 0; i < 2; ++i)
    {
        if (context->mix_bufs[i] != NULL)
        {
            free(context->mix_bufs[i]);
            context->mix_bufs[i] = NULL;
        }    
    }
    
    return;
}
