#pragma once
#include "TrieTreeNodeAbs.h"
class TrieTreeAbs
{
public:
	TrieTreeAbs();
	virtual ~TrieTreeAbs();
	bool operator==(const TrieTreeAbs& that) const;
protected:
	TrieTreeNodeAbs* root;
};

