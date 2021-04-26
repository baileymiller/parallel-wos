#include <pwos/common.h>
#include <pwos/stats.h>

void Stats::reset()
{
    #pragma omp critical
    numClosestPointQueries = 0;

    #pragma omp critical
    numGridQueries = 0;
}

void Stats::initThreadTimers(int nthreads)
{
    threadTime = vector<fsec>(nthreads);
}

void Stats::TIME_THREAD(size_t tid, FunctionBlock f)
{
    THROW_IF(threadTime.size() <= tid, "Must initialize thread timers. Thread " + to_string(tid) + " out of range.");
    auto start = Time::now();
    f();
    threadTime[tid] += Time::now() - start;
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
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "|     Profiling Results                 |" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::cout << "Number of Closest Point Queries:\t\t" << numClosestPointQueries << std::endl;
    std::cout << "Number of Grid Queries:\t\t" << numGridQueries << std::endl;

    if (threadTime.size() > 0) std::cout << "Thread Times:" << std::endl;
    for (int i = 0; i < threadTime.size(); i++)
    {
        std::cout << "\t #" << i << " " << threadTime[i].count() << " s" << std::endl;
    }
}
