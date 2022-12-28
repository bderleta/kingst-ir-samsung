#include "IrSamsungAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>
#include <Windows.h>

IrSamsungAnalyzerSettings::IrSamsungAnalyzerSettings()
    :   mDataChannel(UNDEFINED_CHANNEL),
        mShiftOrder(AnalyzerEnums::LsbFirst),
        mShortPulse(BIT_LOW),
        mLongPulse(BIT_HIGH),
        mDataIsInverted(BIT_HIGH)
{
    mDataChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mDataChannelInterface->SetTitleAndTooltip("Data", "IR Receiver Output");
    mDataChannelInterface->SetChannel(mDataChannel);
    mDataChannelInterface->SetSelectionOfNoneIsAllowed(false);

    mShiftOrderInterface.reset(new AnalyzerSettingInterfaceNumberList());
    mShiftOrderInterface->SetTitleAndTooltip("Byte Order", "");
    mShiftOrderInterface->AddNumber(AnalyzerEnums::LsbFirst, "Least Significant Bit First (Standard)", "");
    mShiftOrderInterface->AddNumber(AnalyzerEnums::MsbFirst, "Most Significant Bit First", "");
    mShiftOrderInterface->SetNumber(mShiftOrder);

    mShortPulseInterface.reset(new AnalyzerSettingInterfaceNumberList());
    mShortPulseInterface->SetTitleAndTooltip("Modulation", "");
    mShortPulseInterface->AddNumber(BIT_LOW, "Short Pulse = 0, Long Pulse = 1", "");
    mShortPulseInterface->AddNumber(BIT_HIGH, "Short Pulse = 1, Long Pulse = 0", "");
    mShortPulseInterface->SetNumber(mShortPulse);

    mDataIsInvertedInterface.reset(new AnalyzerSettingInterfaceBool());
    mDataIsInvertedInterface->SetTitleAndTooltip("", "Bus is by default in high state");
    mDataIsInvertedInterface->SetCheckBoxText("Inverted Bus Level");
    mDataIsInvertedInterface->SetValue(mDataIsInverted);

    AddInterface(mDataChannelInterface.get());
    AddInterface(mShiftOrderInterface.get());
    AddInterface(mShortPulseInterface.get());
    AddInterface(mDataIsInvertedInterface.get());

    AddExportOption(0, "Export as text/csv file");
    AddExportExtension(0, "Text file", "txt");
    AddExportExtension(0, "CSV file", "csv");

    ClearChannels();
    AddChannel(mDataChannel, "DATA", false);
}

IrSamsungAnalyzerSettings::~IrSamsungAnalyzerSettings()
{
}
 
bool IrSamsungAnalyzerSettings::SetSettingsFromInterfaces()
{
    mDataChannel = mDataChannelInterface->GetChannel();
    mShiftOrder = (AnalyzerEnums::ShiftOrder)U32(mShiftOrderInterface->GetNumber());
    mShortPulse = (BitState)U32(mShortPulseInterface->GetNumber());
    mLongPulse = (mShortPulse == BIT_LOW) ? BIT_HIGH : BIT_LOW;
    mDataIsInverted = mDataIsInvertedInterface->GetValue();

    ClearChannels();
    AddChannel(mDataChannel, "DATA", true);

    return true;
}

void IrSamsungAnalyzerSettings::LoadSettings(const char *settings)
{
    SimpleArchive text_archive;
    text_archive.SetString(settings);

    const char *name_string;  //the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if (strcmp(name_string, "DerletaIrSamsungAnalyzer") != 0) {
        AnalyzerHelpers::Assert("Derleta: Provided with a settings string that doesn't belong to us;");
    }

    text_archive >>  mDataChannel;
    text_archive >> *(U32*)&mShiftOrder;
    text_archive >> *(U32*)&mShortPulse;
    mLongPulse = (mShortPulse == BIT_LOW) ? BIT_HIGH : BIT_LOW;

    bool inverted_marker;
    if (text_archive >> inverted_marker) {
        mDataIsInverted = inverted_marker;
    }

    ClearChannels();
    AddChannel(mDataChannel, "DATA", mDataChannel != UNDEFINED_CHANNEL);

    UpdateInterfacesFromSettings();
}

const char *IrSamsungAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "DerletaIrSamsungAnalyzer";
    text_archive <<  mDataChannel;
    text_archive << mShiftOrder;
    text_archive << mShortPulse;
    text_archive << mDataIsInverted;

    return SetReturnString(text_archive.GetString());
}

void IrSamsungAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mDataChannelInterface->SetChannel(mDataChannel);
    mShiftOrderInterface->SetNumber(mShiftOrder);
    mShortPulseInterface->SetNumber(mShortPulse);
    mDataIsInvertedInterface->SetValue(mDataIsInverted);
}