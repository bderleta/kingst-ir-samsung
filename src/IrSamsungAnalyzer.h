#ifndef IR_SAMSUNG_ANALYZER_H
#define IR_SAMSUNG_ANALYZER_H

#include <Analyzer.h>
#include "IrSamsungAnalyzerResults.h"
#include "IrSamsungSimulationDataGenerator.h"

class IrSamsungAnalyzerSettings;

class ANALYZER_EXPORT IrSamsungAnalyzer : public Analyzer
{
public:
    IrSamsungAnalyzer();
    virtual ~IrSamsungAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels);
    virtual U32 GetMinimumSampleRateHz();

    virtual const char *GetAnalyzerName() const;
    virtual bool NeedsRerun();

protected: //functions
    void Setup();
    bool IsInitialClockPolarityCorrect();
    void GetWord();
    int GetByte(U8* value, U64* firstSample, AnalyzerResults::MarkerType startMarker);

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected:  //vars
    std::auto_ptr< IrSamsungAnalyzerSettings > mSettings;
    std::auto_ptr< IrSamsungAnalyzerResults > mResults;
    bool mSimulationInitilized;
    IrSamsungSimulationDataGenerator mSimulationDataGenerator;

    AnalyzerChannelData *mData;

    U64 mCurrentSample;
    AnalyzerResults::MarkerType mArrowMarker;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char *__cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer *__cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer *analyzer);

#endif //IR_SAMSUNG_ANALYZER_H
