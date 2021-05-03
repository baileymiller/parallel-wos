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

    std::cout << "Number of Closest Point Queries: " << numClosestPointQueries << std::endl;
    std::cout << "Number of Grid Queries:" << numGridQueries << std::endl;

    std::cout << "Total time:" << totalTime.count() << std::endl;
    std::cout << "Setup time:" << setupTime.count() << std::endl;

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
    auto [minThreadTime, maxThreadTime] = std::minmax_element(threadTimeF.begin(), threadTimeF.end());
    auto [minQueueTime, maxQueueTime] = std::minmax_element(threadQueueTimeF.begin(), threadQueueTimeF.end());
    auto [minCPGTime, maxCPGTime] = std::minmax_element(threadCPGTimeF.begin(), threadCPGTimeF.end());
    auto [minCPQTime, maxCPQTime] = std::minmax_element(threadCPQTimeF.begin(), threadCPQTimeF.end());

    float avgThreadTime = std::accumulate(threadTimeF.begin(), threadTimeF.end(), 0.0f) / float(nthreads);
    float avgQueueTime = std::accumulate(threadQueueTimeF.begin(), threadQueueTimeF.end(), 0.0f) / float(nthreads);
    float avgCPGTime = std::accumulate(threadCPGTimeF.begin(), threadCPGTimeF.end(), 0.0f) / float(nthreads);
    float avgCPQTime = std::accumulate(threadCPQTimeF.begin(), threadCPQTimeF.end(), 0.0f) / float(nthreads);

    std::cout << "Time per thread: " << "( avg=" << avgThreadTime << ", min=" << *minThreadTime << ", max="<< *maxThreadTime << ")" << std::endl;
    std::cout << "Queue time: " << avgQueueTime << ", min=" << *minQueueTime << ", max="<< *maxQueueTime << ")" << std::endl;
    std::cout << "CP Grid time: " << avgCPGTime << ", min=" << *minCPGTime << ", max="<< *maxCPGTime << ")" << std::endl;
    std::cout << "CP Query time: " << avgCPQTime << ", min=" << *minCPQTime << ", max="<< *maxCPQTime << ")" << std::endl;

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
