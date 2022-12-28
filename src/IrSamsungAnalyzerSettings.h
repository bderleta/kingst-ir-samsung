#ifndef IR_SAMSUNG_ANALYZER_SETTINGS
#define IR_SAMSUNG_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>
 
class IrSamsungAnalyzerSettings : public AnalyzerSettings
{
public:
    IrSamsungAnalyzerSettings();
    virtual ~IrSamsungAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    virtual void LoadSettings(const char *settings);
    virtual const char *SaveSettings();

    void UpdateInterfacesFromSettings();

    Channel mDataChannel;
    AnalyzerEnums::ShiftOrder mShiftOrder;
    BitState mShortPulse;
    BitState mLongPulse;
    bool  mDataIsInverted;

protected:
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mDataChannelInterface;
    std::auto_ptr< AnalyzerSettingInterfaceNumberList > mShiftOrderInterface;
    std::auto_ptr< AnalyzerSettingInterfaceNumberList > mShortPulseInterface;
    std::auto_ptr< AnalyzerSettingInterfaceBool >       mDataIsInvertedInterface;
};

#endif //IR_SAMSUNG_ANALYZER_SETTINGS
