#pragma once
#include "TrieTreeAbs.h"
#include "TrieTreeNodeStatic.h"
class TrieTreeStatic :
	public TrieTreeAbs
{
public:
	TrieTreeStatic(TrieTreeNodeStatic* root);
	virtual ~TrieTreeStatic();
	void setRoot(TrieTreeNodeStatic* newRoot);
};

