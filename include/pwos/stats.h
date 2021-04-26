#pragma once

#include <pwos/common.h>

class Stats
{
public:
    inline static int numClosestPointQueries = 0;
    inline static int numGridQueries = 0;

    static void reset();

    static void INCREMENT_COUNT(StatType type);

    static void report();
};
