#include "IrSamsungAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "IrSamsungAnalyzer.h"
#include "IrSamsungAnalyzerSettings.h"
#include <iostream>
#include <sstream>

#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

IrSamsungAnalyzerResults::IrSamsungAnalyzerResults(IrSamsungAnalyzer* analyzer, IrSamsungAnalyzerSettings* settings)
    : AnalyzerResults(),
    mSettings(settings),
    mAnalyzer(analyzer)
{
}

IrSamsungAnalyzerResults::~IrSamsungAnalyzerResults()
{
}

void IrSamsungAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel &channel, DisplayBase display_base)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    Frame frame = GetFrame(frame_index);
    char number_str[128];

    if (frame.mType == 0x00) {
        AddResultString("Start");
    } 
    else if (frame.mType == 0x01) {
        AddResultString("End");
    }
    else if(frame.mType == 0x02 || frame.mType == 0x03) {
        AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
        AddResultString("Address: ", number_str);
    }
    else if (frame.mType == 0x04 || frame.mType == 0x05) {
        AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
        AddResultString("Command: ", number_str);
    }
}

void IrSamsungAnalyzerResults::GenerateExportFile(const char *file, DisplayBase display_base, U32 /*export_type_user_id*/)
{
    //export_type_user_id is only important if we have more than one export type.

    std::stringstream ss;
    void *f = AnalyzerHelpers::StartFile(file);

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    ss << "Time [s],Packet ID,DATA" << std::endl;

    U64 num_frames = GetNumFrames();
    for (U32 i = 0; i < num_frames; i++) {
        Frame frame = GetFrame(i);

        char time_str[128];
        AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);

        char data_str[128] = "";
        AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
       
        U64 packet_id = GetPacketContainingFrameSequential(i);
        if (packet_id != INVALID_RESULT_INDEX) {
            ss << time_str << "," << packet_id << "," << data_str << std::endl;
        } else {
            ss << time_str << ",," << data_str << "," << std::endl;    //it's ok for a frame not to be included in a packet.
        }

        AnalyzerHelpers::AppendToFile((U8 *)ss.str().c_str(), (U32) ss.str().length(), f);
        ss.str(std::string());

        if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true) {
            AnalyzerHelpers::EndFile(f);
            return;
        }
    }

    UpdateExportProgressAndCheckForCancel(num_frames, num_frames);
    AnalyzerHelpers::EndFile(f);
}

void IrSamsungAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
    ClearTabularText();
    Frame frame = GetFrame(frame_index);

    char data_str[128];

    std::stringstream ss;

    AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, data_str, 128);
    switch (frame.mType) {
        case 0x00:
            ss << "Start"; break;
        case 0x01:
            ss << "End"; break;
        case 0x02:
            ss << "Address: " << data_str; break;
        case 0x03:
            ss << "Rep. Address: " << data_str; break;
        case 0x04:
            ss << "Command: " << data_str; break;
        case 0x05:
            ss << "Inv. Command: " << data_str; break;
        default:
            ss << "Unknown: " << data_str;
    }

    AddTabularText(ss.str().c_str());
}

void IrSamsungAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString("not supported");
}

void IrSamsungAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString("not supported");
}
