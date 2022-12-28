#ifndef IR_SAMSUNG_ANALYZER_RESULTS
#define IR_SAMSUNG_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class IrSamsungAnalyzer;
class IrSamsungAnalyzerSettings;

class IrSamsungAnalyzerResults : public AnalyzerResults
{
public:
    IrSamsungAnalyzerResults(IrSamsungAnalyzer *analyzer, IrSamsungAnalyzerSettings *settings);
    virtual ~IrSamsungAnalyzerResults();

    virtual void GenerateBubbleText(U64 frame_index, Channel &channel, DisplayBase display_base);
    virtual void GenerateExportFile(const char *file, DisplayBase display_base, U32 export_type_user_id);

    virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base);
    virtual void GeneratePacketTabularText(U64 packet_id, DisplayBase display_base);
    virtual void GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base);

protected: //functions

protected: //vars
    IrSamsungAnalyzerSettings *mSettings;
    IrSamsungAnalyzer *mAnalyzer;
};

#endif //IR_SAMSUNG_ANALYZER_RESULTS
