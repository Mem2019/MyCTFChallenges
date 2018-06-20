#pragma once
#include "TrieTreeNode.h"
#include "TrieTreeAbs.h"
class TrieTree : public TrieTreeAbs
{
public:
	TrieTree();
	virtual ~TrieTree();
	void addString(const TrieTreeString& str);
private:
	static void insertString(TrieTreeNode* node, const TrieTreeString& str);
	static size_t numOfCommonPrefix(const TrieTreeString& str1, const TrieTreeString& str2);
};

