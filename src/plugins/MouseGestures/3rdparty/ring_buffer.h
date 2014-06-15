/*
 * This file is part of the mouse gesture package.
 * Copyright (C) 2006 Johan Thelin <e8johan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 *   - Redistributions of source code must retain the above
 *     copyright notice, this list of conditions and the
 *     following disclaimer.
 *   - Redistributions in binary form must reproduce the
 *     above copyright notice, this list of conditions and
 *     the following disclaimer in the documentation and/or
 *     other materials provided with the distribution.
 *   - The names of its contributors may be used to endorse
 *     or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <vector>

/*
* Implementation of Ring Buffer
*/
template<typename T>
class RingBuffer
{
public:
    typedef T*             iterator;
    typedef const T*       const_iterator;

    RingBuffer() {
        array = 0;
        size = 0;
        read = 0;
        write = 0;
        empty = true;
        overflow = false;
    }
    RingBuffer(int size) {
        size = 0;
        read = 0;
        write = 0;
        empty = true;
        overflow = false;
        resize(size);
    }


    void push_back(T item) {
        /*
                if(overflow)
                {
                    throw std::exception("container overflow!");
                }
        */

        array[write++] = item;
        if (write >= size) write = 0;
        empty = false;
        if (write == read) {
            overflow = true;
        }
    }
    T &pop() {
        /*
                if ( empty )
                {
                    throw std::exception("container is empty");
                }
        */
        int tmp = read;
        read++;

        if (read >= size) read = 0;
        overflow = false;
        if (write == read)
            empty = true;
        return array[tmp];
    }

    void setReadPointerTo(int index) {
        read = index;
        if (read >= size) read = 0;
        if (write != read) empty = false;
    }

    int getReadPointer() {
        return read;
    }

    bool is_empty() {
        return empty;
    }

    void resize(int s) {
        size = s;
        array = new T[size];
    }

protected:
    T* array;
    int size;
    int read;
    int write;
    bool overflow;
    bool empty;
};

#endif
