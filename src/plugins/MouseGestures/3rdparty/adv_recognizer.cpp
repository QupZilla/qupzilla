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

#include "adv_recognizer.h"
#include <algorithm>

using namespace Gesture;

RealTimeMouseGestureRecognizer::RealTimeMouseGestureRecognizer(int minimumMovement, double minimumMatch, bool allowDiagonals)
    : minimumMatch(minimumMatch), allowDiagonals(allowDiagonals)
{
    minimumMovement2 = minimumMovement * minimumMovement;

    directions.resize(64);
    lastX = 0;
    lastY = 0;
    lastDirection = NoMatch;
}

RealTimeMouseGestureRecognizer::~RealTimeMouseGestureRecognizer()
{

}


const Direction dirsD[8] = {
    Down,
    Up,
    Right,
    Left,
    DownRight,
    DownLeft,
    UpLeft,
    UpRight
};

void RealTimeMouseGestureRecognizer::addPoint(int x, int y)
{
    int dx, dy;

    dx = x - lastX;
    dy = y - lastY;

    if (dx * dx + dy * dy < minimumMovement2) {
        return;
    }

    Direction direction;

    const int _directions[8][2] = {
        {0, 15}, //down 0
        {0, -15}, //up 1
        {15, 0}, //right 2
        { -15, 0}, //left 3
        {10, 10}, //down right 4
        { -10, 10}, //down left 5
        { -10, -10}, //up left 6
        {10, -10}//up right 7
    };
    int maxValue = 0;
    int maxIndex = -1;

    for (int i = 0; i < (allowDiagonals ? 8 : 4); i++) {
        int value = dx * _directions[i][0] + dy * _directions[i][1];
        if (value > maxValue) {
            maxValue = value;
            maxIndex = i;
        }
    }

    direction = dirsD[maxIndex];

    if (direction != lastDirection) {
        directions.push_back(direction);
        recognizeGesture();
    }


    lastX = x;
    lastY = y;
    lastDirection = direction;
}

struct DirectionSort {
    bool operator()(GestureDefinition a, GestureDefinition b) {
        return a.directions.size() > b.directions.size();
    }
};

void RealTimeMouseGestureRecognizer::addGestureDefinition(const GestureDefinition &gesture)
{
    gestures.push_back(gesture);
    std::sort(gestures.begin(), gestures.end(), DirectionSort());
}

void RealTimeMouseGestureRecognizer::clearGestureDefinitions()
{
    gestures.clear();
}

void RealTimeMouseGestureRecognizer::recognizeGesture()
{
    int first = gestures.size();

    for (GestureList::const_iterator gi = gestures.begin(); gi != gestures.end(); ++gi) {
        int readIndex = directions.getReadPointer();

        try {
            bool match = true;

            for (DirectionList::const_iterator di = gi->directions.begin(); di != gi->directions.end() && match; ++di) {
                Direction d = directions.pop();
                if (*di != d) {
                    match = false;
                }
            }

            if (match) {
                if (gi->callbackClass) {
                    gi->callbackClass->callback();
                }
                return;
            }
            else {
                first--;
                directions.setReadPointerTo(readIndex);
            }
        }
        catch (const std::exception &e) {
            directions.setReadPointerTo(readIndex);
        }
    }

    if (first == 0) {
        directions.pop();
    }
}
