#pragma once
#include "Filter.h"
#include "CircularBuffer.h"

struct DelayInterval
{
CircularBuffer buffer;
Filter filter;
};