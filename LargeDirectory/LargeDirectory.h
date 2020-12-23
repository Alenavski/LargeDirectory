#pragma once

#include "resource.h"
#include <string>

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

typedef BOOL LoadBook(std::wstring* File);
LoadBook* LoadTelephoneBook;

typedef void* SearchRecord(TelephoneRecord* record);
SearchRecord* SearchTelephoneRecord;