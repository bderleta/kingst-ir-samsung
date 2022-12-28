#include "IrSamsungSimulationDataGenerator.h"
#include "IrSamsungAnalyzerSettings.h"

IrSamsungSimulationDataGenerator::IrSamsungSimulationDataGenerator() : 
    mData(NULL), 
    mSettings(NULL), 
    mSimulationSampleRateHz(0), 
    mValue(0)
{
}

IrSamsungSimulationDataGenerator::~IrSamsungSimulationDataGenerator()
{
}
 
void IrSamsungSimulationDataGenerator::Initialize(U32 simulation_sample_rate, IrSamsungAnalyzerSettings*settings)
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mClockGenerator.Init(simulation_sample_rate / 200, simulation_sample_rate);

    if (settings->mDataChannel != UNDEFINED_CHANNEL) {
        mData = mIrSamsungSimulationChannels.Add(settings->mDataChannel, mSimulationSampleRateHz, (settings->mDataIsInverted ? BIT_HIGH : BIT_LOW));
    } else {
        mData = NULL;
    }

    mValue = 0;
}

U32 IrSamsungSimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    while (mData->GetCurrentSampleNumber() < adjusted_largest_sample_requested) {
        CreateIrCommand();

        mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(20.0));  //insert 20 bit-periods of idle
    }

    *simulation_channels = mIrSamsungSimulationChannels.GetArray();
    return mIrSamsungSimulationChannels.GetCount();
}

void IrSamsungSimulationDataGenerator::CreateIrCommand()
{
    mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2.0));

    mData->TransitionIfNeeded(mSettings->mDataIsInverted ? BIT_LOW : BIT_HIGH);
    mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(45e-4));
    mData->TransitionIfNeeded(mSettings->mDataIsInverted ? BIT_HIGH : BIT_LOW);
    mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(45e-4));

    U8 addr = 0b11100000;
    U8 command = 0b01000000;

    OutputByte(addr);
    OutputByte(addr);
    OutputByte(command);
    OutputByte(~command);
}

void IrSamsungSimulationDataGenerator::OutputByte(U8 data_byte)
{
    BitExtractor data_bits(data_byte, mSettings->mShiftOrder, 8);
    BitState nextBit;

    for (U8 i = 0; i < 8; i++) {
        mData->TransitionIfNeeded(mSettings->mDataIsInverted ? BIT_LOW : BIT_HIGH);
        mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(560e-6));
        mData->TransitionIfNeeded(mSettings->mDataIsInverted ? BIT_HIGH : BIT_LOW);
        nextBit = data_bits.GetNextBit();
        if (nextBit == mSettings->mShortPulse) {
            mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(560e-6));
        }
        else if (nextBit == mSettings->mLongPulse) {
            mIrSamsungSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByTimeS(1690e-6));
        }

    }

    mData->TransitionIfNeeded(mSettings->mDataIsInverted ? BIT_HIGH : BIT_LOW);
}
