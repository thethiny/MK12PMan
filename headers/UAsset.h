#pragma once
#define WIN32_LEAN_AND_MEAN
#include "../pch.h"
#include <vector>

struct UAssetHeader {
    uint64_t FilePathFName;
    uint64_t EngineFilesCount; // I assume one of those is a name, the other is how many files are engine but they're sorted so they end up being the same
    uint32_t UFlags;
    uint32_t DataLocationInUCas;
    uint32_t NameTableOffset; // Or HeaderSize
    uint32_t NameTableSize;
    uint32_t ImportDataOffset;
    uint32_t ImportDataSize;
    uint32_t Table0Location;
    uint32_t ExportsLocation; // Size 0x48
    uint32_t Table2Location; // Size 0x20
    uint32_t DataStart;
    uint64_t ExtraDataSize;
};

struct FName {
    uint16_t Length;
    char* String;
};

struct GUID {
    uint32_t Data1; // 32 - 32
    uint16_t Data2; // 16 - 48
    uint16_t Data3; // 16 - 64
    uint8_t  Data4[8]; // 64
};

struct ExportEntry {
    uint64_t ObjectLocation;
    uint64_t ObjectSize;
    uint64_t ObjectName;
    uint64_t UNK1;
    uint64_t UNK2;
    uint64_t UNK3;
    GUID     ObjectClassSignature;
    uint64_t ObjectClass;
};

struct Table2Entry {
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t unk4;
    uint32_t unk5;
    uint32_t unk6;
};

template<typename T>
void printHex(const char* name, T value)
{
    printf("%s: 0x%llX\n", name, value);
}

extern std::vector<FName*> Names;
extern std::vector<ExportEntry*> Exports;
extern std::vector<Table2Entry*> Table2Entries;

void PrintUAssetHeaderDetails(UAssetHeader);
std::vector<FName*> ReadNameTable(const char* data, uint64_t size);
std::vector<ExportEntry*> ReadExportTable(const char* data, uint64_t size);
std::vector<Table2Entry*> ReadTable2(const char* data, uint64_t size);
int ReadData(const char* data, uint64_t size);
int ReadAllData(const char* data, uint64_t size, UAssetHeader header);
const char* IndexToName(uint64_t index);
