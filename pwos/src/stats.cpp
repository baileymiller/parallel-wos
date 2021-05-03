#include <pwos/common.h>
#include <pwos/stats.h>

void Stats::reset()
{
    #pragma omp critical
    numClosestPointQueries = 0;

    #pragma omp critical
    numGridQueries = 0;
}

void Stats::initTimers(int nthreads)
{
    threadTime = vector<fsec>(nthreads);
    threadQueueTime = vector<fsec>(nthreads);
    threadCPGTime = vector<fsec>(nthreads);
    threadCPQTime = vector<fsec>(nthreads);
}

void Stats::TIME(StatTimerType type, FunctionBlock f)
{
    auto start = Time::now();
    f();
    switch (type)
    {
        case StatTimerType::TOTAL:
            totalTime += Time::now() - start;
            break;
        case StatTimerType::SETUP:
            setupTime += Time::now() - start;
            break;
    }
}

void Stats::TIME_THREAD(size_t tid, StatTimerType type, FunctionBlock f)
{
    THROW_IF(threadTime.size() <= tid, "Must initialize thread timers. Thread " + to_string(tid) + " out of range.");
    auto start = Time::now();
    f();
    switch (type)
    {
        case StatTimerType::TOTAL:
            threadTime[tid] += Time::now() - start;
            break;
        case StatTimerType::QUEUE:
            threadQueueTime[tid] += Time::now() - start;
            break;
        case StatTimerType::CLOSEST_POINT_GRID:
            threadCPGTime[tid] += Time::now() - start;
            break;
        case StatTimerType::CLOSEST_POINT_QUERY:
            threadCPQTime[tid] += Time::now() - start;
            break;
    }
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

    std::cout << "\t total time:" << totalTime.count() << std::endl;
    std::cout << "\t setup time:" << setupTime.count() << std::endl;

    // average of thread times
    int nthreads = threadTime.size();
    std::vector<float> threadTimeF, threadQueueTimeF, threadCPGTimeF, threadCPQTimeF;
    for (int i = 0; i < nthreads; i++)
    {
        threadTimeF.push_back(threadTime[i].count());
        threadQueueTimeF.push_back(threadQueueTime[i].count());
        threadCPGTimeF.push_back(threadCPGTime[i].count());
        threadCPQTimeF.push_back(threadCPQTime[i].count());
    }

    float avgThreadTime = std::accumulate(threadTimeF.begin(), threadTimeF.end(), 0.0f) / float(nthreads);
    float avgQueueTime = std::accumulate(threadQueueTimeF.begin(), threadQueueTimeF.end(), 0.0f) / float(nthreads);
    float avgCPGTime = std::accumulate(threadCPGTimeF.begin(), threadCPGTimeF.end(), 0.0f) / float(nthreads);
    float avgCPQTime = std::accumulate(threadCPQTimeF.begin(), threadCPQTimeF.end(), 0.0f) / float(nthreads);

    std::cout << "\t avg time per thread: " << avgThreadTime << std::endl;
    std::cout << "\t avg queue time: " << avgQueueTime << std::endl;
    std::cout << "\t avg CP Grid time: " << avgCPGTime << std::endl;
    std::cout << "\t avg CP Query time: " << avgCPQTime << std::endl;

    // Distribution of thread times
    if (nthreads > 1) 
    {
        std::cout << "Distribution of Thread Times:" << std::endl;
        for (int i = 0; i < threadTime.size(); i++)
        {
            std::cout << "\t #" << i << " " << threadTime[i].count() << " s" << std::endl;
        }
    }
}
