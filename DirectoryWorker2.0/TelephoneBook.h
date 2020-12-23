#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include <memory>

#define SURNAME_INDEX 1
#define STREET_INDEX 2
#define TELEPHONE_INDEX 3

struct TelephoneRecord {
	std::wstring Telephone;
	std::wstring Surname;
	std::wstring Name;
	std::wstring Patronymic;
	std::wstring Street;
	std::wstring House;
	std::wstring Building;
	std::wstring Flat;
};

struct FileMapping {
	HANDLE hFile;
	HANDLE hMapping;
	size_t size;
	DWORD pageSize;
	unsigned char* dataPtr;
} *fileMapping;


extern "C" __declspec(dllexport) BOOL LoadTelephoneBook(std::wstring * File);
extern "C" __declspec(dllexport) void* SearchTelephoneRecord(TelephoneRecord* record);