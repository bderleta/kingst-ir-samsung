#ifndef IR_SAMSUNG_SIMULATION_DATA_GENERATOR
#define IR_SAMSUNG_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class IrSamsungAnalyzerSettings;

class IrSamsungSimulationDataGenerator
{
public:
    IrSamsungSimulationDataGenerator();
    ~IrSamsungSimulationDataGenerator();

    void Initialize(U32 simulation_sample_rate, IrSamsungAnalyzerSettings *settings);
    U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels);

protected:
    IrSamsungAnalyzerSettings*mSettings;
    U32 mSimulationSampleRateHz;
    U64 mValue;

protected: //IR specific
    ClockGenerator mClockGenerator;

    void CreateIrCommand();
    void OutputByte(U8 data_byte);

    SimulationChannelDescriptorGroup mIrSamsungSimulationChannels;
    SimulationChannelDescriptor *mData;
};
#endif //IR_SAMSUNG_SIMULATION_DATA_GENERATOR
