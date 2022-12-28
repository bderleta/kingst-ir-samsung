
#include "IrSamsungAnalyzer.h"
#include "IrSamsungAnalyzerSettings.h"
#include <AnalyzerChannelData.h>


IrSamsungAnalyzer::IrSamsungAnalyzer()
    :   Analyzer(),
        mSettings(new IrSamsungAnalyzerSettings()),
        mSimulationInitilized(false),
        mData(NULL),
        mCurrentSample(0),
        mArrowMarker(AnalyzerResults::MarkerType::ErrorDot)
{
    SetAnalyzerSettings(mSettings.get());
}

IrSamsungAnalyzer::~IrSamsungAnalyzer()
{
    KillThread();
}

void IrSamsungAnalyzer::SetupResults()
{
    mResults.reset(new IrSamsungAnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());

    if (mSettings->mDataChannel != UNDEFINED_CHANNEL) {
        mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel);
    }
}

void IrSamsungAnalyzer::WorkerThread()
{
    Setup();

    mResults->CommitPacketAndStartNewPacket();
    mResults->CommitResults();

    mCurrentSample = mData->GetSampleNumber();

    for (; ;) {
        while (!IsInitialClockPolarityCorrect());
        GetWord();
        CheckIfThreadShouldExit();
    }
}

void IrSamsungAnalyzer::Setup()
{
    if (mSettings->mDataChannel != UNDEFINED_CHANNEL) {
        mData = GetAnalyzerChannelData(mSettings->mDataChannel);
    } else {
        mData = NULL;
    }
}

bool IrSamsungAnalyzer::IsInitialClockPolarityCorrect()
{
    if (mData->GetBitState() == (mSettings->mDataIsInverted ? BIT_HIGH : BIT_LOW)) {
        return true;
    }

    mData->AdvanceToNextEdge();  //at least start with the clock in the idle state.
    mCurrentSample = mData->GetSampleNumber();
    return true;
}

int IrSamsungAnalyzer::GetByte(U8* value, U64* firstSample, AnalyzerResults::MarkerType startMarker)
{
    U8 bits_per_transfer = 8;
    U32 signal_sample_duration = 0;
    DataBuilder data_result;
    U64 data_word = 0;
    data_result.Reset(&data_word, mSettings->mShiftOrder, bits_per_transfer);
    for (U32 i = 0; i < bits_per_transfer; i++) {
        if (i == 0) {
            mResults->AddMarker(mData->GetSampleNumber(), startMarker, mSettings->mDataChannel);
            if (firstSample) *firstSample = mData->GetSampleNumber();
        }
        mCurrentSample = mData->GetSampleNumber();
        if (mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.0004)) || !mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.0008))) {
            mResults->AddMarker(mData->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mDataChannel);
            return (2 * i) + 1;
        }
        mData->AdvanceToNextEdge();
        signal_sample_duration = U32(mData->GetSampleNumber() - mCurrentSample);
        mCurrentSample = mData->GetSampleNumber();
        if (mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.0004)) || !mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.0024))) {
            mResults->AddMarker(mData->GetSampleNumber(), AnalyzerResults::X, mSettings->mDataChannel);
            return (2 * i) + 2;
        }
        data_result.AddBit(mData->WouldAdvancingCauseTransition(signal_sample_duration * 2) ? mSettings->mShortPulse : mSettings->mLongPulse);
        mData->AdvanceToNextEdge();
    }
    *value = U8(data_word & 0xFF);
    return 0;
}

void IrSamsungAnalyzer::GetWord()
{
    //we're assuming we come into this function with the clock in the idle state;
    U64 first_sample = 0;

    U8 addr, raddr, cmd, ncmd;

    ReportProgress(mData->GetSampleNumber());

    mData->AdvanceToNextEdge(); // first edge of preamble
    first_sample = mData->GetSampleNumber();
    if (mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.004)) || !mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.005))) {
        mResults->AddMarker(mData->GetSampleNumber(), mArrowMarker, mSettings->mDataChannel);
        return;
    }
    mData->AdvanceToNextEdge(); // middle edge of preamble
    if (mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.004)) || !mData->WouldAdvancingCauseTransition(U32(this->GetSampleRate() * 0.005))) {
        mResults->AddMarker(mData->GetSampleNumber(), mArrowMarker, mSettings->mDataChannel);
        return;
    }
    mData->AdvanceToNextEdge(); // last edge of preamble; first byte start
    Frame start_frame;
    start_frame.mStartingSampleInclusive = first_sample;
    start_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    start_frame.mType = 0x00;
    mResults->AddFrame(start_frame);


    if (GetByte(&addr, &first_sample, AnalyzerResults::Start)) return;
    Frame addr_frame;
    addr_frame.mStartingSampleInclusive = first_sample;
    addr_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    addr_frame.mData1 = addr;
    addr_frame.mType = 0x02;
    mResults->AddFrame(addr_frame);

    if (GetByte(&raddr, &first_sample, AnalyzerResults::Dot)) return;
    Frame raddr_frame;
    raddr_frame.mStartingSampleInclusive = first_sample;
    raddr_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    raddr_frame.mData1 = raddr;
    raddr_frame.mType = 0x03;
    if (raddr != addr) {
        raddr_frame.mFlags = DISPLAY_AS_ERROR_FLAG;
    }
    mResults->AddFrame(raddr_frame);

    if (GetByte(&cmd, &first_sample, AnalyzerResults::Square)) return;
    Frame cmd_frame;
    cmd_frame.mStartingSampleInclusive = first_sample;
    cmd_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    cmd_frame.mData1 = cmd;
    cmd_frame.mType = 0x04;
    mResults->AddFrame(cmd_frame);

    if (GetByte(&ncmd, &first_sample, AnalyzerResults::Dot)) return;
    Frame ncmd_frame;
    ncmd_frame.mStartingSampleInclusive = first_sample;
    ncmd_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    ncmd_frame.mData1 = (ncmd ^ 0xFF);
    ncmd_frame.mType = 0x05;
    if (cmd != (ncmd ^ 0xFF)) {
        ncmd_frame.mFlags = DISPLAY_AS_ERROR_FLAG;
    }
    mResults->AddMarker(mData->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mDataChannel);
    mResults->AddFrame(ncmd_frame);
    
    first_sample = mData->GetSampleNumber();
    mData->AdvanceToNextEdge();
    Frame end_frame;
    end_frame.mStartingSampleInclusive = first_sample;
    end_frame.mEndingSampleInclusive = mData->GetSampleNumber();
    end_frame.mType = 0x01;
    mResults->AddFrame(end_frame);

    mResults->CommitResults();
}

bool IrSamsungAnalyzer::NeedsRerun()
{
    return false;
}

U32 IrSamsungAnalyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}


U32 IrSamsungAnalyzer::GetMinimumSampleRateHz()
{
    return 10000; //we don't have any idea, depends on the SPI rate, etc.; return the lowest rate.
}

const char *IrSamsungAnalyzer::GetAnalyzerName() const
{
    return "IR-Samsung";
}

const char *GetAnalyzerName()
{
    return "IR-Samsung";
}

Analyzer *CreateAnalyzer()
{
    return new IrSamsungAnalyzer();
}

void DestroyAnalyzer(Analyzer *analyzer)
{
    delete analyzer;
}
