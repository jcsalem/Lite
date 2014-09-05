// Tools for calculating running statistics

#include "utils.h"
#include "utilsStats.h"
#include <cmath>
#include <limits>
#include <sstream>

template<typename T>
StatsCollector<T>::StatsCollector(int numToRetain)
{
  iNumToRetain = numToRetain;
  iFirst = numToRetain > 0 ? new T[numToRetain] : NULL;
  iLast  = numToRetain > 0 ? new T[numToRetain] : NULL;
  iNextLast = 0;
  iMin = numeric_limits<T>::max();
  iMax = numeric_limits<T>::min();
  iCount = 0;
  iSums = 0;
  iSumOfSquares = 0;
}

template<typename T>
StatsCollector<T>::~StatsCollector()
{
  if (iFirst) delete[] iFirst;
  if (iLast)  delete[] iLast;
}

template<typename T>
void StatsCollector<T>::Record(T value)
{
  if (iFirst && iCount < iNumToRetain) iFirst[iCount] = value;
  if (iLast) {
    iLast[iNextLast] = value;
    iNextLast = (iNextLast + 1) % iNumToRetain;
  }
  iSums += value;
  iSumOfSquares += value * value;
  if (value < iMin) iMin = value;
  if (value > iMax) iMax = value;
  ++iCount;
}

template<typename T>
double StatsCollector<T>::GetMean() const
{
  if (iCount > 0) 
    return (double) iSums / (double) iCount;
  else
    return 0;
}

template<typename T>
double StatsCollector<T>::GetStdDev() const
{
  if (iCount == 0) return 0;
  double c = iCount;
  double s = iSums;
  double ss = iSumOfSquares;
  return sqrt(c * ss - s * s) / c;
}

template<typename T>
string StatsCollector<T>::GetOutputString() const
{
  return GetSummaryString() + "\nSamples: " + GetSamplesString();
}

template<typename T>
string StatsCollector<T>::GetSummaryString() const
{
  stringstream stream;
  stream << "Average: " << GetMean() << " ";
  stream << "StdDev: " << GetStdDev() << "  ";
  stream << "Count: " << iCount << "   ";
  if (iCount > 0) {
    stream << "Min/Max: " << iMin << "/" << iMax;
  }
  return stream.str();
}

template<typename T>
string StatsCollector<T>::GetSamplesString() const
{
  stringstream stream;
  for (int i = 0; i < min(iCount, iNumToRetain); ++i)
	  stream << iFirst[i] << " ";
  if (iCount > iNumToRetain) 
    {
	   if (iCount > iNumToRetain * 2)
       stream << "... ";
	   int lastCount = iNumToRetain;
	   int lastStart = iNextLast;
	   if (iCount < iNumToRetain * 2) 
	     {
	     int diff = iNumToRetain * 2 - iCount;
	     lastCount = lastCount - diff;
	     lastStart = (lastStart + diff) % iNumToRetain;
	     }
	   for (int i = 0; i < lastCount; ++i) 
	     stream << iLast[(lastStart + i) % iNumToRetain] << " ";
    }
  return stream.str();
}

// Now force the definition of standard numeric types
template class StatsCollector<long>;
template class StatsCollector<uint32>;

