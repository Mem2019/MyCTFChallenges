#include "TrieTreeNodeStatic.h"


TrieTreeNodeStatic::TrieTreeNodeStatic(): TrieTreeNodeAbs()
{
}


TrieTreeNodeStatic::~TrieTreeNodeStatic()
{
}

void TrieTreeNodeStatic::addChild(TrieTreeNodeStatic* newChild)
{
	children.push_back(newChild);
}
