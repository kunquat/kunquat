

/*
 * This is a modified version of the FFTPACK C implementation released
 * to the public domain, source: http://www.netlib.org/fftpack/fft.c
 *
 * Modifications for Kunquat by Tomi Jylhä-Ollila, Finland 2016
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/fft.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static void drfti1(int32_t n, float* wa, int* ifac);
static void drftf1(int32_t n, float* c, float* ch, float* wa, const int* ifac);
static void drftb1(int32_t n, float* c, float* ch, const float* wa, const int* ifac);


FFT_worker* FFT_worker_init(FFT_worker* worker, int32_t max_tlength)
{
    rassert(worker != NULL);
    rassert(max_tlength > 0);

    worker->wsave = memory_calloc_items(float, max_tlength * 2);
    if (worker->wsave == NULL)
        return NULL;

    worker->max_length = max_tlength;
    worker->cur_length = 0;

    for (int i = 0; i < 32; ++i)
        worker->ifac[0] = 0;

    return worker;
}


static void rfft_init(int32_t n, float* wsave, int* ifac)
{
    rassert(n >= 1);
    rassert(wsave != NULL);
    rassert(ifac != NULL);

    if (n == 1)
        return;

    drfti1(n, wsave + n, ifac);

    return;
}


void FFT_worker_rfft(FFT_worker* worker, float* data, int32_t length)
{
    rassert(worker != NULL);
    rassert(data != NULL);
    rassert(length > 0);
    rassert(length <= worker->max_length);

    if (length != worker->cur_length)
    {
        rfft_init(length, worker->wsave, worker->ifac);
        worker->cur_length = length;
    }

    if (length == 1)
        return;

    drftf1(length, data, worker->wsave, worker->wsave + length, worker->ifac);

    return;
}


void FFT_worker_irfft(FFT_worker* worker, float* data, int32_t length)
{
    rassert(worker != NULL);
    rassert(data != NULL);
    rassert(length > 0);
    rassert(length <= worker->max_length);

    if (length != worker->cur_length)
    {
        rfft_init(length, worker->wsave, worker->ifac);
        worker->cur_length = length;
    }

    if (length == 1)
        return;

    drftb1(length, data, worker->wsave, worker->wsave + length, worker->ifac);

    return;
}


void FFT_worker_deinit(FFT_worker* worker)
{
    rassert(worker != NULL);

    memory_free(worker->wsave);
    worker->wsave = NULL;

    return;
}


static void drfti1(int32_t n, float* wa, int* ifac)
{
    const int ntryh[4] = { 4, 2, 3, 5 };
    int nf = 0;

    // Divide n into preferred set of factors
    int test_index = 0;
    int left = n;
    while (left > 1)
    {
        // Try dividing by numbers in ntryh followed by 7, 9, 11,...
        const int ntry = (test_index < 4) ? ntryh[test_index] : test_index * 2 - 1;

        const int nq = left / ntry;
        const int nr = left - ntry * nq;
        if (nr != 0)
        {
            ++test_index;
            continue;
        }

        nf++;
        ifac[nf + 1] = ntry;
        left = nq;

        if ((ntry == 2) && (nf != 1))
        {
            for (int i = 1; i < nf; ++i)
            {
                const int ib = nf - i + 1;
                ifac[ib + 1] = ifac[ib];
            }
            ifac[2] = 2;
        }
    }

    ifac[0] = n;
    ifac[1] = nf;

    // Fill in complex roots of unity
    const int nfm1 = nf - 1;

    if (nfm1 == 0)
        return;

    const float argh = (float)(PI2 / n);
    int is = 0;
    int l1 = 1;

    for (int k1 = 0; k1 < nfm1; k1++)
    {
        const int ip = ifac[k1 + 2];
        int ld = 0;
        const int l2 = l1 * ip;
        const int ido = n / l2;
        const int ipm = ip - 1;

        for (int j = 0; j < ipm; j++)
        {
            ld += l1;
            int i = is;
            const float argld = (float)ld * argh;
            float fi = 0.0f;
            for (int ii = 2; ii < ido; ii += 2)
            {
                fi += 1.0f;
                const float arg = fi * argld;
                wa[i++] = cosf(arg);
                wa[i++] = sinf(arg);
            }
            is += ido;
        }
        l1 = l2;
    }

    return;
}


static void dradf2(int ido, int l1, float* cc, float* ch, const float* wa1)
{
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;

    t1 = 0;
    t0 = (t2 = l1 * ido);
    t3 = ido << 1;
    for (int k = 0; k < l1; k++)
    {
        ch[t1 << 1] = cc[t1] + cc[t2];
        ch[(t1 << 1) + t3 - 1] = cc[t1] - cc[t2];
        t1 += ido;
        t2 += ido;
    }

    if (ido < 2)
        return;

    if (ido != 2)
    {
        t1 = 0;
        t2 = t0;
        for (int k = 0; k < l1; k++)
        {
            t3 = t2;
            t4 = (t1 << 1) + (ido << 1);
            t5 = t1;
            t6 = t1 + t1;
            for (int i = 2; i < ido; i += 2)
            {
                t3 += 2;
                t4 -= 2;
                t5 += 2;
                t6 += 2;
                const float tr2 = wa1[i - 2] * cc[t3 - 1] + wa1[i - 1] * cc[t3];
                const float ti2 = wa1[i - 2] * cc[t3] - wa1[i - 1] * cc[t3 - 1];
                ch[t6] = cc[t5] + ti2;
                ch[t4] = ti2 - cc[t5];
                ch[t6 - 1] = cc[t5 - 1] + tr2;
                ch[t4 - 1] = cc[t5 - 1] - tr2;
            }
            t1 += ido;
            t2 += ido;
        }

        if (ido % 2 == 1)
            return;
    }

    t3 = (t2 = (t1 = ido) - 1);
    t2 += t0;
    for (int k = 0; k < l1; k++)
    {
        ch[t1] = -cc[t2];
        ch[t1 - 1] = cc[t3];
        t1 += ido << 1;
        t2 += ido;
        t3 += ido;
    }

    return;
}


static void dradf4(
        int ido, int l1, float* cc, float* ch,
        const float* wa1, const float* wa2, const float* wa3)
{
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;

    t0 = l1 * ido;

    t1 = t0;
    t4 = t1 << 1;
    t2 = t1 + (t1 << 1);
    t3 = 0;

    for (int k = 0; k < l1; k++)
    {
        const float tr1 = cc[t1] + cc[t2];
        const float tr2 = cc[t3] + cc[t4];
        ch[t5 = t3 << 2] = tr1 + tr2;
        ch[(ido << 2) + t5 - 1] = tr2 - tr1;
        ch[(t5 += (ido << 1)) - 1] = cc[t3] - cc[t4];
        ch[t5] = cc[t2] - cc[t1];

        t1 += ido;
        t2 += ido;
        t3 += ido;
        t4 += ido;
    }

    if (ido < 2)
        return;

    if (ido != 2)
    {
        t1 = 0;
        for (int k = 0; k < l1; k++)
        {
            t2 = t1;
            t4 = t1 << 2;
            t5 = (t6 = ido << 1) + t4;
            for (int i = 2; i < ido; i += 2)
            {
                t3 = (t2 += 2);
                t4 += 2;
                t5 -= 2;

                t3 += t0;
                const float cr2 = wa1[i - 2] * cc[t3 - 1] + wa1[i - 1] * cc[t3];
                const float ci2 = wa1[i - 2] * cc[t3] - wa1[i - 1] * cc[t3 - 1];
                t3 += t0;
                const float cr3 = wa2[i - 2] * cc[t3 - 1] + wa2[i - 1] * cc[t3];
                const float ci3 = wa2[i - 2] * cc[t3] - wa2[i - 1] * cc[t3 - 1];
                t3 += t0;
                const float cr4 = wa3[i - 2] * cc[t3 - 1] + wa3[i - 1] * cc[t3];
                const float ci4 = wa3[i - 2] * cc[t3] - wa3[i - 1] * cc[t3 - 1];

                const float tr1 = cr2 + cr4;
                const float tr4 = cr4 - cr2;
                const float ti1 = ci2 + ci4;
                const float ti4 = ci2 - ci4;
                const float ti2 = cc[t2] + ci3;
                const float ti3 = cc[t2] - ci3;
                const float tr2 = cc[t2 - 1] + cr3;
                const float tr3 = cc[t2 - 1] - cr3;

                ch[t4 - 1] = tr1 + tr2;
                ch[t4] = ti1 + ti2;

                ch[t5 - 1] = tr3 - ti4;
                ch[t5] = tr4 - ti3;

                ch[t4 + t6 - 1] = ti4 + tr3;
                ch[t4 + t6] = tr4 + ti3;

                ch[t5 + t6 - 1] = tr2 - tr1;
                ch[t5 + t6] = ti1 - ti2;
            }
            t1 += ido;
        }
        if (ido % 2 == 1)
            return;
    }

    t2 = (t1 = t0 + ido - 1) + (t0 << 1);
    t3 = ido << 2;
    t4 = ido;
    t5 = ido << 1;
    t6 = ido;

    const float hsqt2 = 0.70710678118654752440084436210485f;

    for (int k = 0; k < l1; k++)
    {
        const float ti1 = -hsqt2 * (cc[t1] + cc[t2]);
        const float tr1 = hsqt2 * (cc[t1] - cc[t2]);
        ch[t4 - 1] = tr1 + cc[t6 - 1];
        ch[t4 + t5 - 1] = cc[t6 - 1] - tr1;
        ch[t4] = ti1 - cc[t1 + t0];
        ch[t4 + t5] = ti1 + cc[t1 + t0];
        t1 += ido;
        t2 += ido;
        t4 += t3;
        t6 += ido;
    }

    return;
}


static void dradfg(
        int ido, int ip, int l1, int idl1, float* cc, float* c1,
        float* c2, float* ch, float* ch2, float* wa)
{
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    int t6 = 0, t7 = 0, t8 = 0, t9 = 0, t10 = 0;

    const float arg = (float)(PI2 / ip);
    const float dcp = cosf(arg);
    const float dsp = sinf(arg);
    const int ipph = (ip + 1) >> 1;
    const int ipp2 = ip;
    const int idp2 = ido;
    const int nbd = (ido - 1) >> 1;
    t0 = l1 * ido;
    t10 = ip * ido;

    if (ido != 1)
    {
        for (int ik = 0; ik < idl1; ik++)
            ch2[ik] = c2[ik];

        t1 = 0;
        for (int j = 1; j < ip; j++)
        {
            t1 += t0;
            t2 = t1;
            for (int k = 0; k < l1; k++)
            {
                ch[t2] = c1[t2];
                t2 += ido;
            }
        }

        int is = -ido;
        t1 = 0;
        if (nbd > l1)
        {
            for (int j = 1; j < ip; j++)
            {
                t1 += t0;
                is += ido;
                t2 = -ido + t1;
                for (int k = 0; k < l1; k++)
                {
                    int idij = is - 1;
                    t2 += ido;
                    t3 = t2;
                    for (int i = 2; i < ido; i += 2)
                    {
                        idij += 2;
                        t3 += 2;
                        ch[t3 - 1] = wa[idij - 1] * c1[t3 - 1] + wa[idij] * c1[t3];
                        ch[t3] = wa[idij - 1] * c1[t3] - wa[idij] * c1[t3 - 1];
                    }
                }
            }
        }
        else
        {
            for (int j = 1; j < ip; j++)
            {
                is += ido;
                int idij = is - 1;
                t1 += t0;
                t2 = t1;
                for (int i = 2; i < ido; i += 2)
                {
                    idij += 2;
                    t2 += 2;
                    t3 = t2;
                    for (int k = 0; k < l1; k++)
                    {
                        ch[t3 - 1] = wa[idij - 1] * c1[t3 - 1] + wa[idij] * c1[t3];
                        ch[t3] = wa[idij - 1] * c1[t3] - wa[idij] * c1[t3 - 1];
                        t3 += ido;
                    }
                }
            }
        }

        t1 = 0;
        t2 = ipp2 * t0;
        if (nbd < l1)
        {
            for (int j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (int i = 2; i < ido; i += 2)
                {
                    t3 += 2;
                    t4 += 2;
                    t5 = t3 - ido;
                    t6 = t4 - ido;
                    for (int k = 0; k < l1; k++)
                    {
                        t5 += ido;
                        t6 += ido;
                        c1[t5 - 1] = ch[t5 - 1] + ch[t6 - 1];
                        c1[t6 - 1] = ch[t5] - ch[t6];
                        c1[t5] = ch[t5] + ch[t6];
                        c1[t6] = ch[t6 - 1] - ch[t5 - 1];
                    }
                }
            }
        }
        else
        {
            for (int j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (int k = 0; k < l1; k++)
                {
                    t5 = t3;
                    t6 = t4;
                    for (int i = 2; i < ido; i += 2)
                    {
                        t5 += 2;
                        t6 += 2;
                        c1[t5 - 1] = ch[t5 - 1] + ch[t6 - 1];
                        c1[t6 - 1] = ch[t5] - ch[t6];
                        c1[t5] = ch[t5] + ch[t6];
                        c1[t6] = ch[t6 - 1] - ch[t5 - 1];
                    }
                    t3 += ido;
                    t4 += ido;
                }
            }
        }
    }

    for (int ik = 0; ik < idl1; ik++)
        c2[ik] = ch2[ik];

    t1 = 0;
    t2 = ipp2 * idl1;
    for (int j = 1; j < ipph; j++)
    {
        t1 += t0;
        t2 -= t0;
        t3 = t1 - ido;
        t4 = t2 - ido;
        for (int k = 0; k < l1; k++)
        {
            t3 += ido;
            t4 += ido;
            c1[t3] = ch[t3] + ch[t4];
            c1[t4] = ch[t4] - ch[t3];
        }
    }

    float ar1 = 1.0f;
    float ai1 = 0.0f;
    t1 = 0;
    t2 = ipp2 * idl1;
    t3 = (ip - 1) * idl1;
    for (int l = 1; l < ipph; l++)
    {
        t1 += idl1;
        t2 -= idl1;
        const float ar1h = dcp * ar1 - dsp * ai1;
        ai1 = dcp * ai1 + dsp * ar1;
        ar1 = ar1h;
        t4 = t1;
        t5 = t2;
        t6 = t3;
        t7 = idl1;

        for (int ik = 0; ik < idl1; ik++)
        {
            ch2[t4++] = c2[ik] + ar1 * c2[t7++];
            ch2[t5++] = ai1 * c2[t6++];
        }

        const float dc2 = ar1;
        const float ds2 = ai1;
        float ar2 = ar1;
        float ai2 = ai1;

        t4 = idl1;
        t5 = (ipp2 - 1) * idl1;
        for (int j = 2; j < ipph; j++)
        {
            t4 += idl1;
            t5 -= idl1;

            const float ar2h = dc2 * ar2 - ds2 * ai2;
            ai2 = dc2 * ai2 + ds2 * ar2;
            ar2 = ar2h;

            t6 = t1;
            t7 = t2;
            t8 = t4;
            t9 = t5;
            for (int ik = 0; ik < idl1; ik++)
            {
                ch2[t6++] += ar2 * c2[t8++];
                ch2[t7++] += ai2 * c2[t9++];
            }
        }
    }

    t1 = 0;
    for (int j = 1; j < ipph; j++)
    {
        t1 += idl1;
        t2 = t1;
        for (int ik = 0; ik < idl1; ik++)
            ch2[ik] += c2[t2++];
    }

    if (ido >= l1)
    {
        for (int i = 0; i < ido; i++)
        {
            t1 = i;
            t2 = i;
            for (int k = 0; k < l1; k++)
            {
                cc[t2] = ch[t1];
                t1 += ido;
                t2 += t10;
            }
        }
    }
    else
    {
        t1 = 0;
        t2 = 0;
        for (int k = 0; k < l1; k++)
        {
            t3 = t1;
            t4 = t2;
            for (int i = 0; i < ido; i++)
                cc[t4++] = ch[t3++];
            t1 += ido;
            t2 += t10;
        }
    }

    t1 = 0;
    t2 = ido << 1;
    t3 = 0;
    t4 = ipp2 * t0;
    for (int j = 1; j < ipph; j++)
    {
        t1 += t2;
        t3 += t0;
        t4 -= t0;

        t5 = t1;
        t6 = t3;
        t7 = t4;

        for (int k = 0; k < l1; k++)
        {
            cc[t5 - 1] = ch[t6];
            cc[t5] = ch[t7];
            t5 += t10;
            t6 += ido;
            t7 += ido;
        }
    }

    if (ido == 1)
        return;

    if (nbd >= l1)
    {
        t1 = -ido;
        t3 = 0;
        t4 = 0;
        t5 = ipp2 * t0;
        for (int j = 1; j < ipph; j++)
        {
            t1 += t2;
            t3 += t2;
            t4 += t0;
            t5 -= t0;
            t6 = t1;
            t7 = t3;
            t8 = t4;
            t9 = t5;
            for (int k = 0; k < l1; k++)
            {
                for (int i = 2; i < ido; i += 2)
                {
                    const int ic = idp2 - i;
                    cc[i + t7 - 1] = ch[i + t8 - 1] + ch[i + t9 - 1];
                    cc[ic + t6 - 1] = ch[i + t8 - 1] - ch[i + t9 - 1];
                    cc[i + t7] = ch[i + t8] + ch[i + t9];
                    cc[ic + t6] = ch[i + t9] - ch[i + t8];
                }
                t6 += t10;
                t7 += t10;
                t8 += ido;
                t9 += ido;
            }
        }
        return;
    }

    t1 = -ido;
    t3 = 0;
    t4 = 0;
    t5 = ipp2 * t0;
    for (int j = 1; j < ipph; j++)
    {
        t1 += t2;
        t3 += t2;
        t4 += t0;
        t5 -= t0;
        for (int i = 2; i < ido; i += 2)
        {
            t6 = idp2 + t1 - i;
            t7 = i + t3;
            t8 = i + t4;
            t9 = i + t5;
            for (int k = 0; k < l1; k++)
            {
                cc[t7 - 1] = ch[t8 - 1] + ch[t9 - 1];
                cc[t6 - 1] = ch[t8 - 1] - ch[t9 - 1];
                cc[t7] = ch[t8] + ch[t9];
                cc[t6] = ch[t9] - ch[t8];
                t6 += t10;
                t7 += t10;
                t8 += ido;
                t9 += ido;
            }
        }
    }

    return;
}


static void drftf1(int32_t n, float* c, float* ch, float* wa, const int* ifac)
{
    int i = 0, k1 = 0, l1 = 0, l2 = 0;
    int na = 0, kh = 0, nf = 0;
    int ip = 0, iw = 0, ido = 0, idl1 = 0, ix2 = 0, ix3 = 0;

    nf = ifac[1];
    na = 1;
    l2 = n;
    iw = n;

    for (k1 = 0; k1 < nf; k1++)
    {
        kh = nf - k1;
        ip = ifac[kh + 1];
        l1 = l2 / ip;
        ido = n / l2;
        idl1 = ido * l1;
        iw -= (ip - 1) * ido;
        na = 1 - na;

        if (ip == 4)
        {
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            if (na != 0)
                dradf4(ido, l1, ch, c, wa + iw - 1, wa + ix2 - 1, wa + ix3 - 1);
            else
                dradf4(ido, l1, c, ch, wa + iw - 1, wa + ix2 - 1, wa + ix3 - 1);

            l2 = l1;
            continue;
        }

        if (ip == 2)
        {
            if (na == 0)
                dradf2(ido, l1, c, ch, wa + iw - 1);
            else
                dradf2(ido, l1, ch, c, wa + iw - 1);

            l2 = l1;
            continue;
        }

        if (ido == 1)
            na = 1 - na;

        if (na == 0)
        {
            dradfg(ido, ip, l1, idl1, c, c, c, ch, ch, wa + iw - 1);
            na = 1;
        }
        else
        {
            dradfg(ido, ip, l1, idl1, ch, ch, ch, c, c, wa + iw - 1);
            na = 0;
        }

        l2 = l1;
    }

    if (na == 1)
        return;

    for (i = 0; i < n; i++)
        c[i] = ch[i];

    return;
}


static void dradb2(int ido, int l1, float* cc, float* ch, const float* wa1)
{
    int i = 0, k = 0, t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;
    float ti2 = 0, tr2 = 0;

    t0 = l1 * ido;

    t1 = 0;
    t2 = 0;
    t3 = (ido << 1) - 1;
    for (k = 0; k < l1; k++)
    {
        ch[t1] = cc[t2] + cc[t3 + t2];
        ch[t1 + t0] = cc[t2] - cc[t3 + t2];
        t2 = (t1 += ido) << 1;
    }

    if (ido < 2)
        return;

    if (ido != 2)
    {
        t1 = 0;
        t2 = 0;
        for (k = 0; k < l1; k++)
        {
            t3 = t1;
            t5 = (t4 = t2) + (ido << 1);
            t6 = t0 + t1;
            for (i = 2; i < ido; i += 2)
            {
                t3 += 2;
                t4 += 2;
                t5 -= 2;
                t6 += 2;
                ch[t3 - 1] = cc[t4 - 1] + cc[t5 - 1];
                tr2 = cc[t4 - 1] - cc[t5 - 1];
                ch[t3] = cc[t4] - cc[t5];
                ti2 = cc[t4] + cc[t5];
                ch[t6 - 1] = wa1[i - 2] * tr2 - wa1[i - 1] * ti2;
                ch[t6] = wa1[i - 2] * ti2 + wa1[i - 1] * tr2;
            }
            t2 = (t1 += ido) << 1;
        }

        if (ido % 2 == 1)
            return;
    }

    t1 = ido - 1;
    t2 = ido - 1;
    for (k = 0; k < l1; k++)
    {
        ch[t1] = cc[t2] + cc[t2];
        ch[t1 + t0] = -(cc[t2 + 1] + cc[t2 + 1]);
        t1 += ido;
        t2 += ido << 1;
    }

    return;
}


static void dradb3(
        int ido, int l1, float* cc, float* ch, const float* wa1, const float *wa2)
{
    const float taur = -0.5f;
    const float taui = 0.86602540378443864676372317075293618f;
    int i = 0, k = 0;
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    int t6 = 0, t7 = 0, t8 = 0, t9 = 0, t10 = 0;
    float ci2 = 0, ci3 = 0, di2 = 0, di3 = 0, cr2 = 0, cr3 = 0;
    float dr2 = 0, dr3 = 0, ti2 = 0, tr2 = 0;

    t0 = l1 * ido;

    t1 = 0;
    t2 = t0 << 1;
    t3 = ido << 1;
    t4 = ido + (ido << 1);
    t5 = 0;
    for (k = 0; k < l1; k++)
    {
        tr2 = cc[t3 - 1] + cc[t3 - 1];
        cr2 = cc[t5] + (taur * tr2);
        ch[t1] = cc[t5] + tr2;
        ci3 = taui * (cc[t3] + cc[t3]);
        ch[t1 + t0] = cr2 - ci3;
        ch[t1 + t2] = cr2 + ci3;
        t1 += ido;
        t3 += t4;
        t5 += t4;
    }

    if (ido == 1)
        return;

    t1 = 0;
    t3 = ido << 1;
    for (k = 0; k < l1; k++)
    {
        t7 = t1 + (t1 << 1);
        t6 = (t5 = t7 + t3);
        t8 = t1;
        t10 = (t9 = t1 + t0) + t0;

        for (i = 2; i < ido; i += 2)
        {
            t5 += 2;
            t6 -= 2;
            t7 += 2;
            t8 += 2;
            t9 += 2;
            t10 += 2;
            tr2 = cc[t5 - 1] + cc[t6 - 1];
            cr2 = cc[t7 - 1] + (taur * tr2);
            ch[t8 - 1] = cc[t7 - 1] + tr2;
            ti2 =cc[t5] - cc[t6];
            ci2 =cc[t7] + (taur * ti2);
            ch[t8] = cc[t7] + ti2;
            cr3 = taui * (cc[t5 - 1] - cc[t6 - 1]);
            ci3 = taui * (cc[t5] + cc[t6]);
            dr2 = cr2 - ci3;
            dr3 = cr2 + ci3;
            di2 = ci2 + cr3;
            di3 = ci2 - cr3;
            ch[t9 - 1] = wa1[i - 2] * dr2 - wa1[i - 1] * di2;
            ch[t9] = wa1[i - 2] * di2 + wa1[i - 1] * dr2;
            ch[t10 - 1] = wa2[i - 2] * dr3 - wa2[i - 1] * di3;
            ch[t10] = wa2[i - 2] * di3 + wa2[i - 1] * dr3;
        }
        t1 += ido;
    }

    return;
}


static void dradb4(
        int ido, int l1, float* cc, float* ch,
        const float* wa1, const float* wa2, const float* wa3)
{
    const float sqrt2 = 1.4142135623730950488016887242097f;
    int i = 0, k = 0;
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0, t8 = 0;
    float ci2 = 0, ci3 = 0, ci4 = 0, cr2 = 0, cr3 = 0, cr4 = 0;
    float ti1 = 0, ti2 = 0, ti3 = 0, ti4 = 0, tr1 = 0, tr2 = 0, tr3 = 0, tr4 = 0;

    t0 = l1 * ido;

    t1 = 0;
    t2 = ido << 2;
    t3 = 0;
    t6 = ido << 1;
    for (k = 0; k < l1; k++)
    {
        t4 = t3 + t6;
        t5 = t1;
        tr3 = cc[t4 - 1] + cc[t4 - 1];
        tr4 = cc[t4] + cc[t4];
        tr1 = cc[t3] - cc[(t4 += t6) - 1];
        tr2 = cc[t3] + cc[t4 - 1];
        ch[t5] = tr2 + tr3;
        ch[t5 += t0] = tr1 - tr4;
        ch[t5 += t0] = tr2 - tr3;
        ch[t5 += t0] = tr1 + tr4;
        t1 += ido;
        t3 += t2;
    }

    if (ido < 2)
        return;

    if (ido != 2)
    {
        t1 = 0;
        for (k = 0; k < l1; k++)
        {
            t5 = (t4 = (t3 = (t2 = t1 << 2) + t6)) + t6;
            t7 = t1;
            for (i = 2; i < ido; i += 2)
            {
                t2 += 2;
                t3 += 2;
                t4 -= 2;
                t5 -= 2;
                t7 += 2;
                ti1 = cc[t2] + cc[t5];
                ti2 = cc[t2] - cc[t5];
                ti3 = cc[t3] - cc[t4];
                tr4 = cc[t3] + cc[t4];
                tr1 = cc[t2 - 1] - cc[t5 - 1];
                tr2 = cc[t2 - 1] + cc[t5 - 1];
                ti4 = cc[t3 - 1] - cc[t4 - 1];
                tr3 = cc[t3 - 1] + cc[t4 - 1];
                ch[t7 - 1] = tr2 + tr3;
                cr3 = tr2 - tr3;
                ch[t7] = ti2 + ti3;
                ci3 = ti2 - ti3;
                cr2 = tr1 - tr4;
                cr4 = tr1 + tr4;
                ci2 = ti1 + ti4;
                ci4 = ti1 - ti4;

                ch[(t8 = t7 + t0) - 1] = wa1[i - 2] * cr2 - wa1[i - 1]*ci2;
                ch[t8] = wa1[i - 2] * ci2 + wa1[i - 1] * cr2;
                ch[(t8 += t0) - 1] = wa2[i - 2] * cr3 - wa2[i - 1] * ci3;
                ch[t8] = wa2[i - 2] * ci3 + wa2[i - 1] * cr3;
                ch[(t8 += t0) - 1] = wa3[i - 2] * cr4 - wa3[i - 1] * ci4;
                ch[t8] = wa3[i - 2] * ci4 + wa3[i - 1] * cr4;
            }
            t1 += ido;
        }

        if (ido % 2 == 1)
            return;
    }

    t1 = ido;
    t2 = ido << 2;
    t3 = ido - 1;
    t4 = ido + (ido << 1);
    for (k = 0; k < l1; k++)
    {
        t5 = t3;
        ti1 = cc[t1] + cc[t4];
        ti2 = cc[t4] - cc[t1];
        tr1 = cc[t1 - 1] - cc[t4 - 1];
        tr2 = cc[t1 - 1] + cc[t4 - 1];
        ch[t5] = tr2 + tr2;
        ch[t5 += t0] = sqrt2 * (tr1 - ti1);
        ch[t5 += t0] = ti2 + ti2;
        ch[t5 += t0] = -sqrt2 * (tr1 + ti1);

        t3 += ido;
        t1 += t2;
        t4 += t2;
    }

    return;
}


static void dradbg(
        int ido, int ip, int l1, int idl1, float* cc, float* c1,
        float* c2, float* ch, float* ch2, const float* wa)
{
    int idij = 0, ipph = 0, i = 0, j = 0, k = 0, l = 0, ik = 0, is = 0;
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;
    int t7 = 0, t8 = 0, t9 = 0, t10 = 0, t11 = 0, t12 = 0;
    float dc2 = 0, ai1 = 0, ai2 = 0, ar1 = 0, ar2 = 0, ds2 = 0;
    int nbd = 0;
    float dcp = 0, arg = 0, dsp = 0, ar1h = 0, ar2h = 0;
    int ipp2 = 0;

    t10 = ip * ido;
    t0 = l1 * ido;
    arg = (float)(PI2 / ip);
    dcp = cosf(arg);
    dsp = sinf(arg);
    nbd = (ido - 1) >> 1;
    ipp2 = ip;
    ipph = (ip + 1) >> 1;

    if (ido >= l1)
    {
        t1 = 0;
        t2 = 0;
        for (k = 0; k < l1; k++)
        {
            t3 = t1;
            t4 = t2;
            for (i = 0; i < ido; i++)
            {
                ch[t3] = cc[t4];
                t3++;
                t4++;
            }
            t1 += ido;
            t2 += t10;
        }
    }
    else
    {
        t1 = 0;
        for (i = 0; i < ido; i++)
        {
            t2 = t1;
            t3 = t1;
            for (k = 0; k < l1; k++)
            {
                ch[t2] = cc[t3];
                t2 += ido;
                t3 += t10;
            }
            t1++;
        }
    }

    t1 = 0;
    t2 = ipp2 * t0;
    t7 = (t5 = ido << 1);
    for (j = 1; j < ipph; j++)
    {
        t1 += t0;
        t2 -= t0;
        t3 = t1;
        t4 = t2;
        t6 = t5;
        for (k = 0; k < l1; k++)
        {
            ch[t3] = cc[t6 - 1] + cc[t6 - 1];
            ch[t4] = cc[t6] + cc[t6];
            t3 += ido;
            t4 += ido;
            t6 += t10;
        }
        t5 += t7;
    }

    if (ido != 1)
    {
        if (nbd >= l1)
        {
            t1 = 0;
            t2 = ipp2 * t0;
            t7 = 0;
            for (j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;

                t7 += (ido << 1);
                t8 = t7;
                for (k = 0; k < l1; k++)
                {
                    t5 = t3;
                    t6 = t4;
                    t9 = t8;
                    t11 = t8;
                    for (i = 2; i < ido; i += 2)
                    {
                        t5 += 2;
                        t6 += 2;
                        t9 += 2;
                        t11 -= 2;
                        ch[t5 - 1] = cc[t9 - 1] + cc[t11 - 1];
                        ch[t6 - 1] = cc[t9 - 1] - cc[t11 - 1];
                        ch[t5] = cc[t9] - cc[t11];
                        ch[t6] = cc[t9] + cc[t11];
                    }
                    t3 += ido;
                    t4 += ido;
                    t8 += t10;
                }
            }
        }
        else
        {
            t1 = 0;
            t2 = ipp2 * t0;
            t7 = 0;
            for (j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                t7 += (ido << 1);
                t8 = t7;
                t9 = t7;
                for (i = 2; i < ido; i += 2)
                {
                    t3 += 2;
                    t4 += 2;
                    t8 += 2;
                    t9 -= 2;
                    t5 = t3;
                    t6 = t4;
                    t11 = t8;
                    t12 = t9;
                    for (k = 0; k < l1; k++)
                    {
                        ch[t5 - 1] = cc[t11 - 1] + cc[t12 - 1];
                        ch[t6 - 1] = cc[t11 - 1] - cc[t12 - 1];
                        ch[t5] = cc[t11] - cc[t12];
                        ch[t6] = cc[t11] + cc[t12];
                        t5 += ido;
                        t6 += ido;
                        t11 += t10;
                        t12 += t10;
                    }
                }
            }
        }
    }

    ar1 = 1.0f;
    ai1 = 0.0f;
    t1 = 0;
    t9 = (t2 = ipp2 * idl1);
    t3 = (ip - 1) * idl1;
    for (l = 1; l < ipph; l++)
    {
        t1 += idl1;
        t2 -= idl1;

        ar1h = dcp * ar1 - dsp * ai1;
        ai1 = dcp * ai1 + dsp * ar1;
        ar1 = ar1h;
        t4 = t1;
        t5 = t2;
        t6 = 0;
        t7 = idl1;
        t8 = t3;
        for (ik = 0; ik < idl1; ik++)
        {
            c2[t4++] = ch2[t6++] + ar1 * ch2[t7++];
            c2[t5++] = ai1 * ch2[t8++];
        }
        dc2 = ar1;
        ds2 = ai1;
        ar2 = ar1;
        ai2 = ai1;

        t6 = idl1;
        t7 = t9 - idl1;
        for (j = 2; j < ipph; j++)
        {
            t6 += idl1;
            t7 -= idl1;
            ar2h = dc2 * ar2 - ds2 * ai2;
            ai2 = dc2 * ai2 + ds2 * ar2;
            ar2 = ar2h;
            t4 = t1;
            t5 = t2;
            t11 = t6;
            t12 = t7;
            for (ik = 0; ik < idl1; ik++)
            {
                c2[t4++] += ar2 * ch2[t11++];
                c2[t5++] += ai2 * ch2[t12++];
            }
        }
    }

    t1 = 0;
    for (j = 1; j < ipph; j++)
    {
        t1 += idl1;
        t2 = t1;
        for (ik = 0; ik < idl1; ik++)
            ch2[ik] += ch2[t2++];
    }

    t1 = 0;
    t2 = ipp2 * t0;
    for (j = 1; j < ipph; j++)
    {
        t1 += t0;
        t2 -= t0;
        t3 = t1;
        t4 = t2;
        for (k = 0; k < l1; k++)
        {
            ch[t3] = c1[t3] - c1[t4];
            ch[t4] = c1[t3] + c1[t4];
            t3 += ido;
            t4 += ido;
        }
    }

    if (ido != 1)
    {
        if (nbd >= l1)
        {
            t1 = 0;
            t2 = ipp2 * t0;
            for (j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (k = 0; k < l1; k++)
                {
                    t5 = t3;
                    t6 = t4;
                    for (i = 2; i < ido; i += 2)
                    {
                        t5 += 2;
                        t6 += 2;
                        ch[t5 - 1] = c1[t5 - 1] - c1[t6];
                        ch[t6 - 1] = c1[t5 - 1] + c1[t6];
                        ch[t5] = c1[t5] + c1[t6 - 1];
                        ch[t6] = c1[t5] - c1[t6 - 1];
                    }
                    t3 += ido;
                    t4 += ido;
                }
            }
        }
        else
        {
            t1 = 0;
            t2 = ipp2 * t0;
            for (j = 1; j < ipph; j++)
            {
                t1 += t0;
                t2 -= t0;
                t3 = t1;
                t4 = t2;
                for (i = 2; i < ido; i += 2)
                {
                    t3 += 2;
                    t4 += 2;
                    t5 = t3;
                    t6 = t4;
                    for (k = 0; k < l1; k++)
                    {
                        ch[t5 - 1] = c1[t5 - 1] - c1[t6];
                        ch[t6 - 1] = c1[t5 - 1] + c1[t6];
                        ch[t5] = c1[t5] + c1[t6 - 1];
                        ch[t6] = c1[t5] - c1[t6 - 1];
                        t5 += ido;
                        t6 += ido;
                    }
                }
            }
        }
    }

    if (ido == 1)
        return;

    for (ik = 0; ik < idl1; ik++)
        c2[ik] = ch2[ik];

    t1 = 0;
    for (j = 1; j < ip; j++)
    {
        t2 = (t1 += t0);
        for (k = 0; k < l1; k++)
        {
            c1[t2] = ch[t2];
            t2 += ido;
        }
    }

    if (nbd <= l1)
    {
        is = -ido - 1;
        t1 = 0;
        for (j = 1; j < ip; j++)
        {
            is += ido;
            t1 += t0;
            idij = is;
            t2 = t1;
            for (i = 2; i < ido; i += 2)
            {
                t2 += 2;
                idij += 2;
                t3 = t2;
                for (k = 0; k < l1; k++)
                {
                    c1[t3 - 1] = wa[idij - 1] * ch[t3 - 1] - wa[idij] * ch[t3];
                    c1[t3] = wa[idij - 1] * ch[t3] + wa[idij] * ch[t3 - 1];
                    t3 += ido;
                }
            }
        }
        return;
    }

    is = -ido - 1;
    t1 = 0;
    for (j = 1; j < ip; j++)
    {
        is += ido;
        t1 += t0;
        t2 = t1;
        for (k = 0; k < l1; k++)
        {
            idij = is;
            t3 = t2;
            for (i = 2; i < ido; i += 2)
            {
                idij += 2;
                t3 += 2;
                c1[t3 - 1] = wa[idij - 1] * ch[t3 - 1] - wa[idij] * ch[t3];
                c1[t3] = wa[idij - 1] * ch[t3] + wa[idij] * ch[t3 - 1];
            }
            t2 += ido;
        }
    }

    return;
}


static void drftb1(int32_t n, float* c, float* ch, const float* wa, const int* ifac)
{
    int i = 0, k1 = 0, l1 = 0, l2 = 0;
    int na = 0;
    int nf = 0, ip = 0, iw = 0, ix2 = 0, ix3 = 0, ido = 0, idl1 = 0;

    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 1;

    for (k1 = 0; k1 < nf; k1++)
    {
        ip = ifac[k1 + 2];
        l2 = ip * l1;
        ido = n / l2;
        idl1 = ido * l1;
        if (ip == 4)
        {
            ix2 = iw + ido;
            ix3 = ix2 + ido;

            if (na != 0)
                dradb4(ido, l1, ch, c, wa + iw - 1, wa + ix2 - 1, wa + ix3 - 1);
            else
                dradb4(ido, l1, c, ch, wa + iw - 1, wa + ix2 - 1, wa + ix3 - 1);
            na = 1 - na;
        }
        else if (ip == 2)
        {
            if (na != 0)
                dradb2(ido, l1, ch, c, wa + iw - 1);
            else
                dradb2(ido, l1, c, ch, wa + iw - 1);
            na = 1 - na;
        }
        else if (ip == 3)
        {
            ix2 = iw + ido;
            if (na != 0)
                dradb3(ido, l1, ch, c, wa + iw - 1, wa + ix2 - 1);
            else
                dradb3(ido, l1, c, ch, wa + iw - 1, wa + ix2 - 1);
            na = 1 - na;
        }
        else
        {
            if (na != 0)
                dradbg(ido, ip, l1, idl1, ch, ch, ch, c, c, wa + iw - 1);
            else
                dradbg(ido, ip, l1, idl1, c, c, c, ch, ch, wa + iw - 1);
            if (ido == 1)
                na = 1 - na;
        }

        l1 = l2;
        iw += (ip - 1) * ido;
    }

    if (na == 0)
        return;

    for (i = 0; i < n; i++)
        c[i] = ch[i];

    return;
}


