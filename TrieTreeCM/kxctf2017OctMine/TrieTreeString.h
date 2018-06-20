#pragma once

#define MAX_STRING_LEN 128
#include <cstring>
#include <iostream>
class TrieTreeString
{
public:
	TrieTreeString();
	TrieTreeString(char* str);
	bool operator==(const TrieTreeString&) const;
	bool operator!=(const TrieTreeString&) const;
	size_t size() const;
	TrieTreeString substr(size_t pos, size_t len) const;
	inline char operator[](size_t idx) const
	{
		if (idx > this->length)
		{
			std::cerr << "\"operator[]\" access of the TrieTreeString is out of bound!" << std::endl;
			exit(-1);
		}
		return this->str[idx];
	}
private:
	inline bool isStrContainable(char* str)
	{
		return strlen(str) < MAX_STRING_LEN;
	}
	char str[MAX_STRING_LEN];
	size_t length;
};

