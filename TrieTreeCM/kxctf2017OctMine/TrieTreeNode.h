#pragma once
#include "TrieTreeNodeAbs.h"

class TrieTreeNode : public TrieTreeNodeAbs
{
public:
	TrieTreeNode();
	TrieTreeNode(const TrieTreeString& content, int number);
	virtual ~TrieTreeNode();

	void addChild(const TrieTreeString& content, int number);
	void clearChildren_NoDelete();

	void setChildren(const TrieTreeChildren<TrieTreeNode>& newChildren);

	TrieTreeChildren<TrieTreeNode> getChildren();

private:
	void freeAllChildren();
};

