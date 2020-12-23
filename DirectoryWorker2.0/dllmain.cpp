// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "TelephoneBook.h"
#include <fstream>
#include <locale>
#include <algorithm>
#include <sstream>
#include <clocale>

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}



bool compare(std::shared_ptr<TelephoneRecord> t1, std::shared_ptr<TelephoneRecord> t2) {
    if (t1->Surname == t2->Surname) {
        if (t1->Name == t2->Name) {
            if (t1->Patronymic == t2->Patronymic) {
                if (t1->Street == t2->Street) {
                    return t1->House < t2->House;
                }
                else return t1->Street < t2->Street;
            }
            else return t1->Patronymic < t2->Patronymic;
        }
        else return t1->Name < t2->Name;
    }
    else return t1->Surname < t2->Surname;

}

BOOL LoadTelephoneBook(std::wstring* File) {
    std::setlocale(LC_ALL, "ru_RU.");
    fileMapping = new FileMapping;
    HANDLE hFile = CreateFile(File->c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    DWORD size = GetFileSize(hFile, nullptr);
    HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);   
    if (hMapping == nullptr) return false;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD pageSize = si.dwAllocationGranularity;

    fileMapping->hFile = hFile;
    fileMapping->size = size;
    fileMapping->hMapping = hMapping;
    fileMapping->dataPtr = nullptr;
    fileMapping->pageSize = pageSize;

    return true;
}

TelephoneRecord* parseString(std::wstring string) {
    std::wistringstream str(string);
    str.imbue(std::locale("ru_RU.utf8"));
    TelephoneRecord* record = new TelephoneRecord;
    str >> record->Surname >> record->Name >> record->Patronymic >> record->Street;
    str >> record->House >> record->Building >> record->Flat >> record->Telephone;
    return record;
}

bool checkRecord(TelephoneRecord *record, TelephoneRecord *pattern) {
    if (!pattern->Surname.empty() && pattern->Surname != record->Surname) return false;
    if (!pattern->Name.empty() && pattern->Name != record->Name) return false;
    if (!pattern->Patronymic.empty() && pattern->Patronymic != record->Patronymic) return false;
    if (!pattern->Street.empty() && pattern->Street != record->Street) return false;
    if (!pattern->House.empty() && pattern->House != record->House) return false;
    if (!pattern->Building.empty() && pattern->Building != record->Building) return false;
    if (!pattern->Flat.empty() && pattern->Flat != record->Flat) return false;
    if (!pattern->Telephone.empty() && pattern->Telephone != record->Telephone) return false;
    return true;
}

void nextPage(DWORD offset) {
    if (fileMapping->dataPtr != nullptr)
        UnmapViewOfFile(fileMapping->dataPtr);
    fileMapping->dataPtr = (unsigned char*)MapViewOfFile(fileMapping->hMapping, FILE_MAP_READ, 0, offset, fileMapping->pageSize); 
    if (fileMapping->dataPtr == nullptr)
        fileMapping->dataPtr = (unsigned char*)MapViewOfFile(fileMapping->hMapping, FILE_MAP_READ, 0, offset, 0);
    
}

unsigned char readByte(DWORD offset) {
    if (offset % fileMapping->pageSize == 0) nextPage(offset);
    return *(fileMapping->dataPtr + (offset % fileMapping->pageSize));
}

wchar_t readUTFsymbol(DWORD *offset) {
    short first = readByte(*offset);
    (*offset) += 1;
    if ((first >> 6) == 3) {
        short second = readByte(*offset);
        (*offset) += 1;
        if ((second >> 6) == 2)
            return (((first & 0x003F) << 6) | (second & 0x003F));
    }
    return first;
}

std::wstring readLine(DWORD* offset) {
    std::wstring result = L"";
    wchar_t symbol;
    do {
        symbol = readUTFsymbol(offset);
        result += symbol;

    } while (symbol != '\n' && ((*offset) < fileMapping->size));
    return result;
}

void* SearchTelephoneRecord(TelephoneRecord *pattern) {
    std::vector<std::shared_ptr<TelephoneRecord>> *result = new std::vector<std::shared_ptr<TelephoneRecord>>;
    DWORD offset = 0;
    while (offset != fileMapping->size) {
        std::wstring line = readLine(&offset);
        TelephoneRecord *record = parseString(line);
        if (checkRecord(record, pattern))
            result->push_back(std::shared_ptr<TelephoneRecord>(record));
        else
            free(record);
    }
    
    return result;
}