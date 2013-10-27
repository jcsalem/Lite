// Support for the LSparkle type
// Controls sparkle

#include "LSparkle.h"
#include "utilsRandom.h"
#include "utilsOptions.h"
#include <algorithm> // for min and max

//----------------------------------------------------------------------
// LSparkle member functions
//----------------------------------------------------------------------

RGBColor LSparkle::ComputeColor(const RGBColor& referenceColor, Milli_t currentTime) const {
//    cout << "Color: " << referenceColor.ToString() << " start: " << startTime << " currentTime: " << currentTime << "  attack: " << attack << "  hold: " << hold << endl;

    if (MilliGE(startTime, currentTime))
        return BLACK;
    else if (MilliGT(startTime + attack, currentTime))
        return referenceColor * ((float)(currentTime - startTime) / attack);
    else if (MilliGE(startTime + attack + hold, currentTime))
        return referenceColor;
    else if (MilliGT(startTime + attack + hold + release, currentTime))
        return referenceColor * ((float) (release - (currentTime - startTime - attack - hold)) / release);
    else
        return BLACK;
}

Milli_t LSparkle::GetEndTime() const {
    return startTime + attack + hold + release + sleep;
}

//bool LobjSparkle::IsOutOfTime() const {
//    return sparkle.IsOutOfTime(lastTime);
//}

//----------------------------------------------------------------------
// LSparkle Global Option definitions
//----------------------------------------------------------------------

LSparkle::Mode_t L::gSparkleMode = LSparkle::kSparkle;

string SparkleCallback(csref name, csref val) {
    L::gSparkleMode = LSparkle::StrToMode(val);
    if (L::gSparkleMode == LSparkle::kError)
        return "--sparkle must be either sparkle, slow or firefly. Was " + val;
    return "";
}
string SparkleDefaultCallback(csref name) {
    return LSparkle::ModeToStr(L::gSparkleMode);
}

DefOption(sparkle, SparkleCallback, "sparklemode", "may be Slow, Sparkle, or Firefly", SparkleDefaultCallback);

float L::gSparkleRate = 1.0;

string SparkleRateCallback(csref name, csref val) {
    if (!StrToFlt(val, &L::gSparkleRate) || L::gSparkleRate <= 0)
        return "--sparklerate must be a number greater than zero. Was " + val;
    return "";
}

string SparkleRateDefaultCallback(csref name) {
    return FltToStr(L::gSparkleRate);
}

DefOption(sparklerate, SparkleRateCallback, "sparklerate",
          "the relative speed of the sparkle going on/off. Only applies to Slow option.",
          SparkleRateDefaultCallback);

//----------------------------------------------------------------------
// LSparkle Global Definitions
//----------------------------------------------------------------------

LSparkle::Mode_t LSparkle::StrToMode(csref str) {
    if      (StrEQ(str, "sparkle"))     return LSparkle::kSparkle;
    else if (StrEQ(str, "firefly"))     return LSparkle::kFirefly;
    else if (StrEQ(str, "slow"))        return LSparkle::kSlow;
    else return LSparkle::kError;
}

string LSparkle::ModeToStr(Mode_t mode) {
    switch (mode) {
        case LSparkle::kSparkle:    return "sparkle";
        case LSparkle::kFirefly:    return "firefly";
        case LSparkle::kSlow:       return "slow";
        case LSparkle::kError:      return "error";
        default:                    return "unknown";
    }
}

LSparkle LSparkle::MakeRandomSparkle (Milli_t currentTime, Mode_t mode, float rate, bool isFirstTime) {
    LSparkle si;

    switch (mode) {
        case LSparkle::kSlow:
            si.attack   = RandomInt(1000/rate);
            si.hold     = RandomMax(3, 10000, 100000)/rate;
            si.release  = RandomInt(1500/rate);
            si.sleep    = RandomNormalBounded(333, 250, 50, 10000);
            break;
        case LSparkle::kFirefly:
            si.attack   = RandomInt(100, 450);
            si.hold     = si.attack * 2 + RandomInt(si.attack) + RandomInt(si.attack);
            si.release  = RandomMin(2, 0, 800) + 400;
            si.sleep    = RandomMin(3, 750, 3000);
            break;
        case LSparkle::kSparkle:
        default:
            si.attack   = min(RandomInt(100), RandomInt(150));
            si.hold     = si.attack/2 + RandomInt(200);
            si.release  = 10;
            si.sleep    = 100;
            break;
    }

    if (isFirstTime) {
        si.startTime = currentTime - RandomInt(si.GetEndTime());
    } else {
        si.startTime = currentTime;
    }

    return si;
}

