#pragma once

#include "TrieTreeChildren.h"
#include "TrieTreeString.h"
using namespace std;

class TrieTreeNodeAbs
{
public:
	TrieTreeNodeAbs();
	TrieTreeNodeAbs(const TrieTreeString& content, int number);
	virtual ~TrieTreeNodeAbs();

	int getNumber();
	TrieTreeString getContent();
	TrieTreeChildren<> getChildren();
	bool operator== (const TrieTreeNodeAbs& that) const;
	bool operator!= (const TrieTreeNodeAbs& that) const;

	void setNumber(int num);
	void setContent(const TrieTreeString& newContent);

protected:
	TrieTreeString content;
	TrieTreeChildren<> children;
	int number;
private:
	TrieTreeNodeAbs* findChildGivenContent(const TrieTreeString& str) const;
};

