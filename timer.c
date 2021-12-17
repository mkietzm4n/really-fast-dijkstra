#include "timer.h"
struct timeval timerStart;

void StartTimer(){
  gettimeofday(&timerStart, NULL);
}

double GetTimer(){
  struct timeval timerStop, timerElapsed;
  gettimeofday(&timerStop, NULL);
  timersub(&timerStop, &timerStart, &timerElapsed);
  return timerElapsed.tv_sec*1000.0+timerElapsed.tv_usec/1000.0;
}