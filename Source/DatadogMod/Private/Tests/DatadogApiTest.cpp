#include "DatadogApi.h"


static FString PrintArray(TArray<uint8>& array) {
    return FString::FromHexBlob(array.GetData(), array.Num());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatadogApiTest, "DatadogMod.DatadogApiTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDatadogApiTest::RunTest(const FString& Parameters)
{
    FString testString = FString("{\"key\": \"value\"}");
    TArray<uint8> expected = TArray<uint8>({ 0x78, 0x9c, 0xab, 0x56, 0xca, 0x4e, 0xad, 0x54, 0xb2, 0x52, 0x50, 0x2a, 0x4b, 0xcc, 0x29, 0x4d, 0x55, 0xaa, 0x05, 0x00, 0x2b, 0xaf, 0x05, 0x41 });
    TArray<uint8> compressed = UDatadogApi::Compress(testString);
    
    // Make the test pass by returning true, or fail by returning false.
    if (compressed != expected) {
        FString error = "Expected " + PrintArray(expected) + ", Got " + PrintArray(compressed);
        AddError(error);
        return false;
    }
    return true;
}
