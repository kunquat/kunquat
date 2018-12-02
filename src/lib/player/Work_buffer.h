

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_WORK_BUFFER_H
#define KQT_WORK_BUFFER_H


#include <decl.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define WORK_BUFFER_ELEM_SIZE ((int)sizeof(float))
#define WORK_BUFFER_SUB_COUNT_MAX 4

#define WORK_BUFFER_SIZE_MAX \
    (((INT32_MAX / WORK_BUFFER_ELEM_SIZE) / WORK_BUFFER_SUB_COUNT_MAX) - 3)


/**
 * Create a new Work buffer.
 *
 * \param size        The buffer size -- must be > \c 0 and
 *                    <= \c WORK_BUFFER_SIZE_MAX.
 * \param sub_count   The number of interleaved areas in the Work buffer --
 *                    must be >= \c 1, <= \c WORK_BUFFER_SUB_COUNT_MAX
 *                    and a power of two.
 *
 * \return   The new Work buffer if successful, or \c NULL if memory allocation
 *           failed.
 */
Work_buffer* new_Work_buffer(int32_t size, int sub_count);


/**
 * Initialise a Work buffer with externally allocated space.
 *
 * NOTE: Work buffers created with \a new_Work_buffer must not be passed as a
 *       parameter to this function, and Work buffers initialised with this
 *       function must not be passed to \a del_Work_buffer.
 *
 * \param buffer           The Work buffer -- must not be \c NULL and must be
 *                         aligned to \c 32 bytes.
 * \param sub_count        The number of interleaved areas in the Work buffer
 *                         -- must be >= \c 1, <= \c WORK_BUFFER_SUB_COUNT_MAX
 *                         and a power of two.
 * \param space            The starting address of the memory area
 *                         -- must not be \c NULL and must be aligned to
 *                         \c 64 bytes.
 * \param raw_elem_count   The total number of elements in \a space -- must be
 *                         >= \a sub_count * \c 2 and divisible by
 *                         \a sub_count. The reported size of the initialised
 *                         Work buffer will be
 *                         (\a raw_elem_count / \a sub_count) - \c 3.
 */
void Work_buffer_init_with_memory(
        Work_buffer* buffer, int sub_count, void* space, int32_t raw_elem_count);


/**
 * Resize the Work buffer.
 *
 * \param buffer     The Work buffer -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be > \c 0 and
 *                   <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size);


/**
 * Set the number of interleaved areas in the Work buffer.
 *
 * Note that this function cannot be used to increase the initial number of
 * interleaved areas in the buffer. Also note that changing the sub_count
 * invalidates the contents of the buffer.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_count   The new number of interleaved areas in the Work buffer --
 *                    must be >= \c 1, <= \c WORK_BUFFER_SUB_COUNT_MAX, a
 *                    power of two and not greater than the initial sub_count
 *                    of \a buffer.
 */
void Work_buffer_set_sub_count(Work_buffer* buffer, int sub_count);


/**
 * Get the number of interleaved areas in the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The number of interleaved areas.
 */
int Work_buffer_get_sub_count(const Work_buffer* buffer);


/**
 * Get the element stride of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The element stride.
 */
int Work_buffer_get_stride(const Work_buffer* buffer);


/**
 * Invalidate the contents of the Work buffer.
 *
 * Work buffers that have invalid contents will automatically clear themselves
 * before mixed into.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 */
void Work_buffer_invalidate(Work_buffer* buffer);


/**
 * Mark subarea of the Work buffer as valid.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area to be marked -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(\a buffer).
 */
void Work_buffer_mark_valid(Work_buffer* buffer, int sub_index);


/**
 * Get the valid status of the Work buffer.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area to be checked -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(\a buffer).
 *
 * \return   \c true if \a buffer at \a sub_index has valid contents, otherwise
 *           \c false.
 */
bool Work_buffer_is_valid(const Work_buffer* buffer, int sub_index);


/**
 * Clear the Work buffer with floating-point zeroes.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area to be cleared -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(\a buffer).
 * \param buf_start   The start index of the area to be cleared -- must be
 *                    >= \c 0 and less than the buffer size.
 * \param buf_stop    The stop index of the area to be cleared -- must be
 *                    >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_clear(
        Work_buffer* buffer, int sub_index, int32_t buf_start, int32_t buf_stop);


/**
 * Clear all areas of Work buffer with floating-point zeroes.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param buf_start   The start index of the area to be cleared -- must be
 *                    >= \c 0 and less than the buffer size.
 * \param buf_stop    The stop index of the area to be cleared -- must be
 *                    >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_clear_all(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop);


/**
 * Get the size of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The size of the buffer.
 */
int32_t Work_buffer_get_size(const Work_buffer* buffer);


/**
 * Get the contents of the Work buffer.
 *
 * Addresses of each element of the returned area is separated by
 * \a Work_buffer_get_stride(buffer) elements.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [0, Work_buffer_get_size(\a buffer) + 3]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
const float* Work_buffer_get_contents(const Work_buffer* buffer, int sub_index)
#ifdef __GNUC__
    __attribute__((assume_aligned(64)))
#endif
    ;


/**
 * Get the mutable contents of the Work buffer.
 *
 * Addresses of each element of the returned area is separated by
 * \a Work_buffer_get_stride(buffer) elements.
 *
 * Note: This function clears the const start index and final status of the
 *       selected area of the buffer as it no longer makes any assumptions of
 *       the buffer contents. If you wish to utilise these optimisation
 *       features, retrieve the state first by calling
 *       \a Work_buffer_get_const_start and \a Work_buffer_is_final.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [0, Work_buffer_get_size(\a buffer) + 3]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
float* Work_buffer_get_contents_mut(Work_buffer* buffer, int sub_index)
#ifdef __GNUC__
    __attribute__((assume_aligned(64)))
#endif
    ;


/**
 * Get the mutable contents of the Work buffer as integer data.
 *
 * Addresses of each element of the returned area is separated by
 * \a Work_buffer_get_stride(buffer) elements.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [0, Work_buffer_get_size(\a buffer) + 3]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
int32_t* Work_buffer_get_contents_int_mut(Work_buffer* buffer, int sub_index)
#ifdef __GNUC__
    __attribute__((assume_aligned(64)))
#endif
    ;


/**
 * Copy contents of the Work buffer into another.
 *
 * \param dest             The destination Work buffer -- must not be \c NULL.
 * \param dest_sub_index   The index of the destination area -- must be >= \c 0
 *                         and < \a Work_buffer_get_sub_count(dest).
 * \param src              The source Work buffer -- must not be \c NULL or \a dest.
 * \param src_sub_index    The index of the source area -- must be >= \c 0 and
 *                         < \a Work_buffer_get_sub_count(src).
 * \param buf_start        The start index of the area to be copied -- must be
 *                         >= \c 0 and less than the buffer size.
 * \param buf_stop         The stop index of the area to be copied -- must be
 *                         >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_copy(
        Work_buffer* restrict dest,
        int dest_sub_index,
        const Work_buffer* restrict src,
        int src_sub_index,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Copy all subareas of a Work buffer into another.
 *
 * \param dest        The destination Work buffer -- must not be \c NULL.
 * \param src         The source Work buffer -- must not be \c NULL or \a dest.
 * \param buf_start   The start index of the area to be copied -- must be
 *                    >= \c 0 and less than the buffer size.
 * \param buf_stop    The stop index of the area to be copied -- must be
 *                    >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_copy_all(
        Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Mark a trailing part of Work buffer contents as having constant value.
 *
 * NOTE: This is used as an optional performance optimisation. The caller
 * must still fill the marked buffer area with constant values for code
 * that does not take advantage of this information.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 * \param start       The start index in \a buffer -- must be non-negative.
 */
void Work_buffer_set_const_start(Work_buffer* buffer, int sub_index, int32_t start);


/**
 * Clear Work buffer constant-value part marker.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 */
void Work_buffer_clear_const_start(Work_buffer* buffer, int sub_index);


/**
 * Get Work buffer constant-value part marker.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 *
 * \return   The constant-value start marker, or \c INT32_MAX if not set.
 */
int32_t Work_buffer_get_const_start(const Work_buffer* buffer, int sub_index);


/**
 * Mark the constant trail of the buffer as final value.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 * \param is_final    Whether or not the trailing constant is the final value.
 */
void Work_buffer_set_final(Work_buffer* buffer, int sub_index, bool is_final);


/**
 * Get the final status of the Work buffer.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param sub_index   The index of the area -- must be >= \c 0 and
 *                    < \a Work_buffer_get_sub_count(buffer).
 *
 * \return   \c true if the constant trail of \a buffer is final, otherwise \c false.
 */
bool Work_buffer_is_final(const Work_buffer* buffer, int sub_index);


/**
 * Mix the contents of a Work buffer into another as floating-point data.
 *
 * If the two buffers are the same Work buffer, this function does nothing.
 *
 * \param dest             The Work buffer that will contain the end result --
 *                         must not be \c NULL.
 * \param dest_sub_index   The index of the destination area -- must be >= \c 0
 *                         and < \a Work_buffer_get_sub_count(dest).
 * \param src              The input Work buffer -- must not be \c NULL and must
 *                         have the same size as \a dest.
 * \param src_sub_index    The index of the destination area -- must be >= \c 0
 *                         and < \a Work_buffer_get_sub_count(src).
 * \param buf_start        The start index of the area to be mixed -- must be
 *                         >= \c 0 and less than the buffer size.
 * \param buf_stop         The stop index of the area to be mixed -- must be
 *                         >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_mix(
        Work_buffer* dest,
        int dest_sub_index,
        const Work_buffer* src,
        int src_sub_index,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Mixes all subareas of a Work buffer into another as floating-point data.
 *
 * If the two buffers are the same Work buffer, this function does nothing.
 *
 * \param dest        The Work buffer that will contain the end result --
 *                    must not be \c NULL.
 * \param src         The input Work buffer -- must not be \c NULL and must
 *                    have the same size and stride as \a dest.
 * \param buf_start   The start index of the area to be mixed -- must be
 *                    >= \c 0 and less than the buffer size.
 * \param buf_stop    The stop index of the area to be mixed -- must be
 *                    >= \c 0 and less than or equal to buffer size.
 */
void Work_buffer_mix_all(
        Work_buffer* dest,
        const Work_buffer* src,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Destroy an existing Work buffer.
 *
 * \param buffer   The Work buffer, or \c NULL.
 */
void del_Work_buffer(Work_buffer* buffer);


#endif // KQT_WORK_BUFFER_H


