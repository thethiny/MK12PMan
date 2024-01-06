#include "UAsset.h"

#pragma warning(disable: 26451)

std::vector<FName*> Names;
std::vector<ExportEntry*> Exports;
std::vector<Table2Entry*> Table2Entries;

const char* IndexToName(uint64_t index)
{
    return Names[index]->String;
}

void PrintUAssetHeaderDetails(UAssetHeader FileHeader)
{
    if (Names.empty())
    {
        printf("Header Info (0x%llX)\n", sizeof(FileHeader));
        printHex("FilePathFName", FileHeader.FilePathFName);
        printHex("EngineFilesCount", FileHeader.EngineFilesCount);
        printHex("UFlags", FileHeader.UFlags);
        printHex("DataLocationInUCas", FileHeader.DataLocationInUCas);
        printHex("NameTableOffset", FileHeader.NameTableOffset);
        printHex("NameTableSize", FileHeader.NameTableSize);
        printHex("ImportDataOffset", FileHeader.ImportDataOffset);
        printHex("ImportDataSize", FileHeader.ImportDataSize);
        printHex("Table0Location", FileHeader.Table0Location);
        printHex("ExportsLocation", FileHeader.ExportsLocation);
        printHex("Table2Location", FileHeader.Table2Location);
        printHex("DataStart", FileHeader.DataStart);
        printHex("ExtraDataSize", FileHeader.ExtraDataSize);
        printf("\n");
    }
    else {
        printf("Header Info (0x%llX)\n", sizeof(FileHeader));
        printf("FilePathFName: 0x%llX [%s]\n", FileHeader.FilePathFName, IndexToName(FileHeader.FilePathFName));
        printHex("EngineFilesCount", FileHeader.EngineFilesCount);
        printHex("UFlags", FileHeader.UFlags);
        printHex("DataLocationInUCas", FileHeader.DataLocationInUCas);
        printHex("NameTableOffset", FileHeader.NameTableOffset);
        printHex("NameTableSize", FileHeader.NameTableSize);
        printHex("ImportDataOffset", FileHeader.ImportDataOffset);
        printHex("ImportDataSize", FileHeader.ImportDataSize);
        printHex("Table0Location", FileHeader.Table0Location);
        printHex("ExportsLocation", FileHeader.ExportsLocation);
        printHex("Table2Location", FileHeader.Table2Location);
        printHex("DataStart", FileHeader.DataStart);
        printHex("ExtraDataSize", FileHeader.ExtraDataSize);
        printf("\n");
    }
}


std::vector<FName*> ReadNameTable(const char* data, uint64_t size)
{
    std::vector<FName*> Names;
    uint64_t i = 0;
    while (i < size)
    {
        FName* CurrentName = new FName;
        Names.push_back(CurrentName);

        CurrentName->Length = *(uint16_t*)(&data[i]);
        CurrentName->Length = (CurrentName->Length << 8) + (CurrentName->Length >> 8);
        CurrentName->String = new char[CurrentName->Length + 1];

        i += sizeof(CurrentName->Length);
        strncpy(CurrentName->String, &data[i], CurrentName->Length);
        CurrentName->String[CurrentName->Length] = '\0';

        i += CurrentName->Length;
        printf("Found Name | %s [0x%llX]\n", CurrentName->String, CurrentName->Length);
    }

    return Names;
}

void printGUID(GUID myGuid)
{
    printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        myGuid.Data1, myGuid.Data2, myGuid.Data3,
        myGuid.Data4[0], myGuid.Data4[1], myGuid.Data4[2], myGuid.Data4[3],
        myGuid.Data4[4], myGuid.Data4[5], myGuid.Data4[6], myGuid.Data4[7]);
}

std::vector<ExportEntry*> ReadExportTable(const char* data, uint64_t size)
{
    std::vector<ExportEntry*> Exports;
    uint64_t i = 0;
    while (i < size)
    {
        ExportEntry* CurrentExport = new ExportEntry;
        Exports.push_back(CurrentExport);
        std::memcpy(CurrentExport, &data[i], sizeof(ExportEntry));
        i += sizeof(ExportEntry);

        printf("Found Export | %s.%s [%llX | %llX | %llX] [",
            IndexToName(CurrentExport->ObjectName), IndexToName(CurrentExport->ObjectClass),
            CurrentExport->UNK1, CurrentExport->UNK2, CurrentExport->UNK3
        );
        printGUID(CurrentExport->ObjectClassSignature);
        printf("]\n");
    }

    return Exports;
}

std::vector<Table2Entry*> ReadTable2(const char* data, uint64_t size)
{
    std::vector<Table2Entry*> Entries;
    uint64_t i = 0;
    while (i < size)
    {
        Table2Entry* CurrentEntry = new Table2Entry;
        Entries.push_back(CurrentEntry);
        std::memcpy(CurrentEntry, &data[i], sizeof(Table2Entry));
        i += sizeof(Table2Entry);

        printf("Found Entry | %lX %lX %lX %lX %lX %lX\n",
            CurrentEntry->unk1, CurrentEntry->unk2, CurrentEntry->unk3,
            CurrentEntry->unk4, CurrentEntry->unk5, CurrentEntry->unk6
        );
    }

    return Entries;
}

int ReadAllData(const char* data, uint64_t size, UAssetHeader FileHeader)
{
    for (int i = 0; i < Exports.size(); i++)
    {
        ExportEntry* curExport = Exports[i];
        uint64_t curSize = curExport->ObjectSize;
        uint64_t curLoc = curExport->ObjectLocation - FileHeader.DataLocationInUCas;
        ReadData(data + curLoc, curSize);
    }

    return 1;
}

int ReadData(const char* data, uint64_t size)
{
    uint64_t i = 0;
    while (i < size)
    {
        const char* type = IndexToName(*(uint64_t*)data+1);
        i += 8;
        const char* subType = IndexToName(*(uint64_t*)(data+i));
        i += 8;
        printf("Type %s SubType %s\n", type, subType);
        uint8_t count = *(uint8_t*)(data + i);
        i = size; // To Force Quit
    }
    return 1;
}