// Tools for calculating running statistics

#ifndef __UTILS_STATS
#define __UTILS_STATS

// This works as long as T is a numeric type
template<typename T>
class StatsCollector
{
 public:
  StatsCollector (int numToRetain = 15);
  ~StatsCollector();
  void Record(T value);
  string GetSummaryString() const;
  string GetSamplesString() const;
  string GetOutputString() const;  // Summary and Samples
  double GetMean() const;
  double GetStdDev() const;
  T GetMin() const {return iMin;}
  T GetMax() const {return iMax;}

 private:
  int  iNumToRetain;
  T*   iFirst;
  T*   iLast;
  int  iNextLast;
  T    iMin;
  T    iMax;
  
  int  iCount;
  T    iSums;
  T    iSumOfSquares;
};

#endif
