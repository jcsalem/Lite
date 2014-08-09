// Tools for calculating running statistics

#ifndef __UTILS_STATS
#define __UTILS_STATS

#include <iostream>

// This should be changed to a template to support any numeric type
class StatsCollector
{
 public:
  StatsCollector (int numToRetain = 5);
  ~StatsCollector();
  void Record(long value);
  void Output(ostream& stream = cout) const;
  double GetMean() const;
  double GetStdDev() const;

 private:
  int   iNumToRetain;
  long* iFirst;
  long* iLast;
  int   iNextLast;

  int   iCount;
  long  iSums;
  long  iSumOfSquares;
};

#endif
