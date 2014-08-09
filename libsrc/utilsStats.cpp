// Tools for calculating running statistics

#include "utils.h"
#include "utilsStats.h"
#include <cmath>

StatsCollector::StatsCollector(int numToRetain)
{
  iNumToRetain = numToRetain;
  iFirst = numToRetain > 0 ? new long[numToRetain] : NULL;
  iLast  = numToRetain > 0 ? new long[numToRetain] : NULL;
  iNextLast = 0;
  iCount = 0;
  iSums = 0;
  iSumOfSquares = 0;
}

StatsCollector::~StatsCollector()
{
  if (iFirst) delete[] iFirst;
  if (iLast)  delete[] iLast;
}

void StatsCollector::Record(long value)
{
  if (iFirst && iCount < iNumToRetain) iFirst[iCount] = value;
  if (iLast) {
    iLast[iNextLast] = value;
    iNextLast = (iNextLast + 1) % iNumToRetain;
  }
  iSums += value;
  iSumOfSquares += value * value;
  ++iCount;
}

double StatsCollector::GetMean() const
{
  if (iCount > 0) 
    return (double) iSums / (double) iCount;
  else
    return 0;
}

double StatsCollector::GetStdDev() const
{
  if (iCount == 0) return 0;
  double c = iCount;
  double s = iSums;
  double ss = iSumOfSquares;
  return sqrt(c * ss - s * s) / c;
}

void StatsCollector::Output(ostream& stream) const
{
  stream << "Average: " << GetMean() << "  ";
  stream << "StdDev: " << GetStdDev() << "  ";
  stream << "Count: " << iCount;
  stream << endl;
  if (iNumToRetain > 0 && iCount > 0) {
    stream << "Samples: ";
    for (int i = 0; i < min(iCount, iNumToRetain); ++i)
	stream << iFirst[i] << " ";
    if (iCount > iNumToRetain) 
      {
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
    stream << endl;
  }
}
