#include <pwos/common.h>
#include <pwos/stats.h>

void Stats::init(int nthreads)
{
    threadTime = vector<fsec>(nthreads);
    threadSendWalksTime = vector<fsec>(nthreads);
    threadRecvWalksTime = vector<fsec>(nthreads);
    threadCPGTime = vector<fsec>(nthreads);
    threadCPQTime = vector<fsec>(nthreads);
    threadCPQSetupTime = vector<fsec>(nthreads);
    numClosestPointQueries = vector<int>(nthreads);
    numGridQueries = vector<int>(nthreads);
    numClosestPointQueriesSetup = vector<int>(nthreads);
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
        case StatTimerType::GRID_CREATION:
            gridCreationTime += Time::now() - start;
            break;
    }
}

void Stats::TIME_THREAD(StatTimerType type, FunctionBlock f)
{
    size_t tid = omp_get_thread_num();
    THROW_IF(threadTime.size() <= tid, "Must initialize thread timers. Thread " + to_string(tid) + " out of range.");
    auto start = Time::now();
    f();
    switch (type)
    {
        case StatTimerType::TOTAL:
            threadTime[tid] += Time::now() - start;
            break;
        case StatTimerType::SEND_WALKS:
            threadSendWalksTime[tid] += Time::now() - start;
            break;
     case StatTimerType::RECV_WALKS:
            threadRecvWalksTime[tid] += Time::now() - start;
            break;
        case StatTimerType::CLOSEST_POINT_GRID:
            threadCPGTime[tid] += Time::now() - start;
            break;
        case StatTimerType::CLOSEST_POINT_QUERY:
            threadCPQTime[tid] += Time::now() - start;
            break;
        case StatTimerType::SETUP_CLOSEST_POINT_QUERY:
            threadCPQSetupTime[tid] += Time::now() - start;
            break;
    }
}

void Stats::SET_COUNT(StatType type, int val)
{
    switch(type)
    {
        case StatType::GRID_POINTS:
            #pragma omp critical
            numGridPoints = val;
            break;
    }
}

void Stats::INCREMENT_COUNT(StatType type)
{
    size_t tid = omp_get_thread_num();
    switch(type)
    {
        case StatType::CLOSEST_POINT_QUERY:
            numClosestPointQueries[tid] += 1;
            break;
        case StatType::SETUP_CLOSEST_POINT_QUERY:
            numClosestPointQueriesSetup[tid] += 1;
            break;
        case StatType::GRID_QUERY:
            numGridQueries[tid] += 1;
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
    int nthreads = threadTime.size();

    int totalCPQ = 0;
    int totalSetupCPQ = 0;
    int totalGQ = 0;
    for (int i = 0; i < nthreads; i++)
    {
        totalCPQ += numClosestPointQueries[i];
        totalSetupCPQ += numClosestPointQueriesSetup[i];
        totalGQ += numGridQueries[i];
    }

    std::cout << "Number of Closest Point Queries: " << totalCPQ << std::endl;
    std::cout << "Number of Closest Point Queries during Setup: " << totalSetupCPQ << std::endl;
    std::cout << "Number of Grid Queries:" << totalGQ << std::endl;
    std::cout << "Number of Grid Points" << numGridPoints << std::endl;
    std::cout << "Total time:" << totalTime.count() << std::endl;
    std::cout << "Setup time:" << setupTime.count() << std::endl;
    std::cout << "Grid Creation Time:" << gridCreationTime.count() << std::endl;

    // average of thread times
    std::vector<float> threadTimeF, threadSendWalksTimeF, threadRecvWalksTimeF, threadCPGTimeF, threadCPQTimeF, threadCPQSetupTimeF;
    float totalSendTime = 0, totalRecvTime = 0, totalCPGTime = 0, totalCPQTime = 0, totalCPQSetupTime = 0;
    for (int i = 0; i < nthreads; i++)
    {
        threadTimeF.push_back(threadTime[i].count());
        threadSendWalksTimeF.push_back(threadSendWalksTime[i].count());
        threadRecvWalksTimeF.push_back(threadRecvWalksTime[i].count());
        threadCPGTimeF.push_back(threadCPGTime[i].count());
        threadCPQTimeF.push_back(threadCPQTime[i].count());
        threadCPQSetupTimeF.push_back(threadCPQSetupTime[i].count());

        totalSendTime += threadSendWalksTime[i].count();
        totalRecvTime += threadRecvWalksTime[i].count();
        totalCPGTime += threadCPGTime[i].count();
        totalCPQTime += threadCPQTime[i].count();
        totalCPQSetupTime += threadCPQSetupTime[i].count();
    }
    std::cout << "Total Send Time:" << totalSendTime << " s" << std::endl;
    std::cout << "Total Recv Time:" << totalRecvTime << " s" << std::endl;
    std::cout << "Total CPG Time:" << totalCPGTime << " s" << std::endl;
    std::cout << "Total CPQ Time:" << totalCPQTime << " s" << std::endl;
    std::cout << "Total CPQ During Setup Time:" << totalCPQSetupTime << " s" << std::endl;

    std::cout << "Avg CPQ Time" << (totalCPQSetupTime + totalCPQTime) / float(totalCPQ + totalSetupCPQ) << " s" << std::endl;
    std::cout << "Avg GQ Time" << (totalCPGTime) / float(totalGQ) << " s" << std::endl;

    auto [minThreadTime, maxThreadTime] = std::minmax_element(threadTimeF.begin(), threadTimeF.end());
    auto [minSendWalksTime, maxSendWalksTime] = std::minmax_element(threadSendWalksTimeF.begin(), threadSendWalksTimeF.end());
    auto [minRecvWalksTime, maxRecvWalksTime] = std::minmax_element(threadRecvWalksTimeF.begin(), threadRecvWalksTimeF.end());
    auto [minCPGTime, maxCPGTime] = std::minmax_element(threadCPGTimeF.begin(), threadCPGTimeF.end());
    auto [minCPQTime, maxCPQTime] = std::minmax_element(threadCPQTimeF.begin(), threadCPQTimeF.end());
    auto [minCPQSetupTime, maxCPQSetupTime] = std::minmax_element(threadCPQSetupTimeF.begin(), threadCPQSetupTimeF.end());

    float avgThreadTime = std::accumulate(threadTimeF.begin(), threadTimeF.end(), 0.0f) / float(nthreads);
    float avgSendWalksTime = std::accumulate(threadSendWalksTimeF.begin(), threadSendWalksTimeF.end(), 0.0f) / float(nthreads);
    float avgRecvWalksTime = std::accumulate(threadRecvWalksTimeF.begin(), threadRecvWalksTimeF.end(), 0.0f) / float(nthreads);
    float avgCPGTime = std::accumulate(threadCPGTimeF.begin(), threadCPGTimeF.end(), 0.0f) / float(nthreads);
    float avgCPQTime = std::accumulate(threadCPQTimeF.begin(), threadCPQTimeF.end(), 0.0f) / float(nthreads);
    float avgCPQSetupTime = std::accumulate(threadCPQSetupTimeF.begin(), threadCPQSetupTimeF.end(), 0.0f) / float(nthreads);

    std::cout << "Time per thread: ( avg=" << avgThreadTime << ", min=" << *minThreadTime << ", max="<< *maxThreadTime << ")" << std::endl;
    std::cout << "Send Walks time: ( avg=" << avgSendWalksTime << ", min=" << *minSendWalksTime << ", max="<< *maxSendWalksTime << ")" << std::endl;
    std::cout << "Recv Walks time: ( avg=" << avgRecvWalksTime << ", min=" << *minRecvWalksTime << ", max="<< *maxRecvWalksTime << ")" << std::endl;
    std::cout << "CP Grid time: ( avg=" << avgCPGTime << ", min=" << *minCPGTime << ", max="<< *maxCPGTime << ")" << std::endl;
    std::cout << "CP Query time: ( avg=" << avgCPQTime << ", min=" << *minCPQTime << ", max="<< *maxCPQTime << ")" << std::endl;
    std::cout << "CP Query time during Setup: ( avg=" << avgCPQSetupTime << ", min=" << *minCPQSetupTime << ", max="<< *maxCPQSetupTime << ")" << std::endl;

    // Distribution of thread times
    if (nthreads > 1) 
    {
        std::cout << "Distribution of Thread Time, CPQs, and GQs:" << std::endl;
        for (int i = 0; i < threadTime.size(); i++)
        {
            std::cout << "\t Thread #" << i << std::endl;;
            std::cout << "\t\t\t\t Total Time=" << threadTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t Send Time=" << threadSendWalksTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t Recv Time=" << threadRecvWalksTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t CPQ Time=" << threadCPQTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t CPQ Setup Time=" << threadCPQSetupTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t GQ Time=" << threadCPGTime[i].count() << " s" << std::endl;;
            std::cout << "\t\t\t\t CPQs=" << numClosestPointQueries[i] << " s" << std::endl;;
            std::cout << "\t\t\t\t CPQs Setup=" << numClosestPointQueriesSetup[i] << " s" << std::endl;;
            std::cout << "\t\t\t\t GQs=" << numGridQueries[i] << std::endl;
        }
    }
}
