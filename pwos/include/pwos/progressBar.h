#pragma once

#include <pwos/common.h>

class ProgressBar {
public:
  int barWidth = 70;

  int workCompleted = 0;
  int totalWork = 0;

  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

  ProgressBar() {}
  ProgressBar(int barWidth): barWidth(barWidth) {}

  void start() {
    draw(0.0f);
    startTime = Time::now();
  }

  void start(int _totalWork)
  {
    workCompleted = 0;
    totalWork = _totalWork;
    start();
  }

  void set(float progress) {
    draw(progress);
  }

  ProgressBar operator++(int)
  {
    workCompleted++;
    set(float(workCompleted) / totalWork);
    return *this;
  }
  void operator++()
  {
    workCompleted ++;
    set(float(workCompleted) / totalWork);
  }

  void finish() {
    draw(1.0f);
    std::cout << std::endl;
    fsec duration = Time::now() - startTime;
    std::cout << "Finished in " <<  duration.count() << " s" << std::endl;
  }

private:
  void draw(float progress) {
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout << std::flush;
  }
};
