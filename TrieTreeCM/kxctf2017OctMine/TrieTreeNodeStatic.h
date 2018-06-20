#pragma once

#include "TrieTreeNodeAbs.h"

class TrieTreeNodeStatic : public TrieTreeNodeAbs
{
public:
	TrieTreeNodeStatic();
	virtual ~TrieTreeNodeStatic();
	void addChild(TrieTreeNodeStatic* newChild);
};

