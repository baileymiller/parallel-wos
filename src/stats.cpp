#include <pwos/common.h>
#include <pwos/stats.h>

void Stats::reset()
{
    #pragma omp atomic
    numClosestPointQueries = 0;

    #pragma omp atomic
    numGridQueries = 0;
}

void Stats::INCREMENT_COUNT(StatType type)
{
    switch(type)
    {
        case StatType::CLOSEST_POINT_QUERY:
            #pragma omp atomic
            numClosestPointQueries++;
            break;
        case StatType::GRID_QUERY:
            #pragma omp atomic
            numGridQueries++;
            break;
        default:
            break;
    }
}

void Stats::report()
{
    std::cout << "Profiling Results:" << std::endl;
    std::cout << "Number of Closest Point Queries:\t\t" << numClosestPointQueries << std::endl;
    std::cout << "Number of Grid Queries:\t\t" << numGridQueries << std::endl;
}
