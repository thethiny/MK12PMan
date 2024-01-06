#include <iostream>
#include <fstream>
#include <cstdio>
#include <list>
#include <filesystem>

#include "headers/UAsset.h"

using namespace std;
namespace fs = std::filesystem;

void extractAndWriteTable(const string& tableName, uint64_t startOffset, uint64_t size, const std::string& outFolder, ifstream& fin)
{
    std::string tableOut = outFolder + "\\" + tableName + ".uAsset";
    std::ofstream outFile(tableOut.c_str(), std::ios::binary);

    char* rwbuff = new char[size];
    //fin.seekg(startOffset, std::ios::beg); // Don't need this 
    fin.read(rwbuff, size);

    printf("Extracting Header Table 0x%llx -> %s [0x%llX]\n", startOffset, tableName.c_str(), size);
    printf("Writing %s [RAW] to %s\n", tableName.c_str(), tableOut.c_str());

    outFile.write(rwbuff, size);
    delete[] rwbuff;
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        cerr << "Missing file name!";
        return 1;
    }

    uint64_t i = 0;
    
    printf("Processing %s...\n", argv[1]);
    std::string OutFolder = argv[1];
    OutFolder += "_extract";
    std::string OutRawFolder = OutFolder + "\\Raw";
    std::string OutParsedFolder = OutFolder + "\\Parsed";

    if (!fs::exists(OutRawFolder)) {
        if (!fs::create_directories(OutRawFolder)) {
            std::cerr << "Error creating directory: " << OutRawFolder << std::endl;
            return 2;
        }
    }

    if (!fs::exists(OutParsedFolder)) {
        if (!fs::create_directories(OutParsedFolder)) {
            std::cerr << "Error creating directory: " << OutParsedFolder << std::endl;
            return 2;
        }
    }

    ifstream fin(argv[1], ios::binary | ios::ate);
    fin >> noskipws;
    if (!fin)
    {
        cerr << "Error opening file!";
        return 2;
    }

    streamsize size = fin.tellg();  // Get the size of the file
    fin.seekg(0, ios::beg);  // Move the file pointer back to the beginning
    printf("Reading %i (0x%llX) bytes...\n", size, size);

    UAssetHeader FileHeader;

    fin.read((char*)&FileHeader, sizeof(FileHeader));

    PrintUAssetHeaderDetails(FileHeader);

    printf("Extracting Header Table 0x%llX -> NameTable [0x%llX]\n", FileHeader.NameTableOffset, FileHeader.NameTableSize);

    char* NameTable = new char[FileHeader.NameTableSize];
    fin.read(NameTable, FileHeader.NameTableSize);


    std::string NameTableOut = OutRawFolder + "\\NameTable.uAsset";

    ofstream ofNameTable(NameTableOut.c_str(), ios::binary);

    printf("Writing Name Table [RAW] to %s...\n", NameTableOut.c_str());

    ofNameTable.write(NameTable, FileHeader.NameTableSize);
    ofNameTable.close();

    printf("Parsing Name Table...\n");
    Names = ReadNameTable(NameTable, FileHeader.NameTableSize);
    printf("Parsed %d names...\n", Names.size());

    delete[] NameTable;

    NameTableOut = OutParsedFolder + "\\NameTable.txt";
    printf("Writing Name Table [Parsed] to %s...\n", NameTableOut.c_str());
    ofstream ofNameTableP(NameTableOut.c_str());

    for (int i = 0; i < Names.size(); i++)
    {
        char Buff[500];
        sprintf(Buff, "%llX: %s\n", i, Names[i]->String);
        ofNameTableP << Buff;
    }


    PrintUAssetHeaderDetails(FileHeader);

    // Skip padding
    if (fin.tellg() != FileHeader.ImportDataOffset)
    {
        printf("Padding detected. Skipping %llX bytes...\n", FileHeader.ImportDataOffset - fin.tellg());
    }
    fin.seekg(FileHeader.ImportDataOffset, ios::beg);


    char* rwbuff;
    uint64_t readSize;

    printf("Extracting Header Table 0x%llX -> ImportTable [0x%llX]\n", FileHeader.ImportDataOffset, FileHeader.ImportDataSize);
    string ImportTableOut = OutRawFolder + "\\ImportTable.uAsset";
    ofstream ofImportTable(ImportTableOut.c_str(), ios::binary);
    readSize = FileHeader.ImportDataSize;
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Import Table [RAW] to %s\n", ImportTableOut.c_str());
    ofImportTable.write(rwbuff, readSize);
    delete[] rwbuff;


    readSize = FileHeader.ExportsLocation - FileHeader.Table0Location;
    printf("Extracting Header Table 0x%llX -> Table0 [0x%llX]\n", FileHeader.Table0Location, readSize);
    string Table0Out = OutRawFolder + "\\Table0.uAsset";
    ofstream ofTab0(Table0Out.c_str(), ios::binary);
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Table 0 [RAW] to %s\n", Table0Out.c_str());
    ofTab0.write(rwbuff, readSize);
    delete[] rwbuff;


    readSize = FileHeader.Table2Location - FileHeader.ExportsLocation;
    printf("Extracting Header Table 0x%llX -> ExportTable [0x%llX]\n", FileHeader.ExportsLocation, readSize);
    string ExportTableOut = OutRawFolder + "\\ExportTable.uAsset";
    ofstream ofExportTable(ExportTableOut.c_str(), ios::binary);
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Export Table [RAW] to %s\n", ExportTableOut.c_str());
    ofExportTable.write(rwbuff, readSize);

    printf("Parsing Export Table...\n");
    Exports = ReadExportTable(rwbuff, readSize);
    printf("Parsed %d exports...\n", Exports.size());

    delete[] rwbuff;

    ExportTableOut = OutParsedFolder + "\\ExportsTable.txt";
    printf("Writing Name Table [Parsed] to %s...\n", ExportTableOut.c_str());
    ofstream ofExportsP(ExportTableOut.c_str());

    for (int i = 0; i < Exports.size(); i++)
    {
        char Buff[500];
        sprintf(Buff, "%llX: %s (%llX) [%llX | %llX | %llX]", i,
            IndexToName(Exports[i]->ObjectName), Exports[i]->ObjectClass,
            Exports[i]->UNK1, Exports[i]->UNK2, Exports[i]->UNK3
        );
        ofExportsP << Buff;
        GUID myGuid = Exports[i]->ObjectClassSignature;
        sprintf(Buff, " [%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X]\n",
            myGuid.Data1, myGuid.Data2, myGuid.Data3,
            myGuid.Data4[0], myGuid.Data4[1], myGuid.Data4[2], myGuid.Data4[3],
            myGuid.Data4[4], myGuid.Data4[5], myGuid.Data4[6], myGuid.Data4[7]);
        ofExportsP << Buff;
    }


    readSize = FileHeader.DataStart - FileHeader.Table2Location;;
    printf("Extracting Header Table 0x%llX -> Table2 [0x%llX]\n", FileHeader.Table2Location, readSize);
    string Table2Out = OutRawFolder + "\\Table2.uAsset";
    ofstream ofTable2(Table2Out.c_str(), ios::binary);
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Table 2 [RAW] to %s\n", Table2Out.c_str());
    ofTable2.write(rwbuff, readSize);

    printf("Parsing Table 2...\n");
    Table2Entries = ReadTable2(rwbuff, readSize);
    printf("Parsed %d entries...\n", Table2Entries.size());

    delete[] rwbuff;

    Table2Out = OutParsedFolder + "\\Table2.txt";
    printf("Writing Table 2 [Parsed] to %s...\n", Table2Out.c_str());
    ofstream ofTable2P(Table2Out.c_str());

    for (int i = 0; i < Table2Entries.size(); i++)
    {
        char Buff[500];
        Table2Entry* CurrentEntry = Table2Entries[i];
        sprintf(Buff, "%llX: %lX %lX %lX %lX %lX %lX\n", i,
            CurrentEntry->unk1, CurrentEntry->unk2, CurrentEntry->unk3,
            CurrentEntry->unk4, CurrentEntry->unk5, CurrentEntry->unk6
        );

        ofTable2P << Buff;
    }

    readSize = FileHeader.ExtraDataSize;
    printf("Extracting Extra Data 0x%llX [0x%llX]\n", FileHeader.DataStart, FileHeader.ExtraDataSize);
    string ExtraOut = OutRawFolder + "\\ExtraData.uAsset";
    ofstream ofExtra(ExtraOut.c_str(), ios::binary);
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Extra Data [RAW] to %s\n", ExtraOut.c_str());
    ofExtra.write(rwbuff, readSize);

    delete[] rwbuff;

    auto curIdx = fin.tellg();
    fin.seekg(0, ios::end);
    auto fileEnd = fin.tellg();
    fin.seekg(curIdx, ios::beg);
    readSize = fileEnd - curIdx;
    printf("Extracting Data 0x%llX [0x%llX]\n", FileHeader.DataStart + FileHeader.ExtraDataSize, readSize);
    string DataOut = OutRawFolder + "\\Data.uAsset";
    ofstream ofData(DataOut.c_str(), ios::binary);
    rwbuff = new char[readSize];
    fin.read(rwbuff, readSize);
    printf("Writing Data [RAW] to %s\n", DataOut.c_str());
    ofData.write(rwbuff, readSize);

    // Write Extracted Data
    printf("Extracting Data...\n");
    for (int i = 0; i < Exports.size(); i++)
    {
        ExportEntry* curExport = Exports[i];
        uint64_t curSize = curExport->ObjectSize;
        uint64_t curLoc = curExport->ObjectLocation - FileHeader.DataLocationInUCas;

        char buff[512];
        sprintf(buff, "%d_%s_%llx", i, IndexToName(curExport->ObjectName), curExport->ObjectClass);
        std::string oName = OutRawFolder + "\\Extracted";
        if (!fs::exists(oName)) {
            if (!fs::create_directories(oName)) {
                std::cerr << "Error creating directory: " << oName << std::endl;
                return 2;
            }
        }
        oName += "\\";
        oName += buff;
        ofstream dataOut(oName.c_str(), ios::binary);
        printf("Writing file [EXTRACT] %s of type %llX to %s", IndexToName(curExport->ObjectName), curExport->ObjectClass, IndexToName(curExport->ObjectClass), oName.c_str());
        dataOut.write(rwbuff + curLoc, curSize);
    }

    printf("Parsing Data...\n");
    auto DataEntries = ReadAllData(rwbuff, readSize, FileHeader);
    printf("Parsed %d entries...\n", Table2Entries.size());
    delete[] rwbuff;

}