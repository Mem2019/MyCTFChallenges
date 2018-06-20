#include "TrieTreeString.h"
#include <cstring>
#include <iostream>
using namespace std;
TrieTreeString::TrieTreeString()
{
	length = 0;
	str[0] = '\0';
}

TrieTreeString::TrieTreeString(char* str)
{
	if (!isStrContainable(str))
	{
		//cerr << "No no no, the string length exceeds the maximum string length in trie tree node" << endl;
		exit(-1);
	}
	strcpy_s(this->str, MAX_STRING_LEN, str);
	this->length = strlen(str);
}

size_t TrieTreeString::size() const
{
	return length;
}

TrieTreeString TrieTreeString::substr(size_t pos, size_t len) const
{
	if (pos >= length)
	{
		//cerr << "invalid \"pos\" argument for method substr of class TrieTreeString!" << endl;
		exit(-1);
	}
	char* buffer = new char[this->length];
	size_t idx = 0;
	for (size_t i = pos; i < length && idx < len; i++, idx++)
	{
		buffer[idx] = this->str[i];
	}
	buffer[idx] = 0;
	TrieTreeString ret = TrieTreeString(buffer);
	delete[] buffer;
	return ret;
}

bool TrieTreeString::operator==(const TrieTreeString& that) const
{
	return strcmp(this->str, that.str) == 0;
}

bool TrieTreeString::operator!=(const TrieTreeString& that) const
{
	return !(*this == that);
}

