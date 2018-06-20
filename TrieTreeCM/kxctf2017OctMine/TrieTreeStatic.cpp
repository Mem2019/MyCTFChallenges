#include "stdafx.h"
#include "TrieTreeStatic.h"


TrieTreeStatic::TrieTreeStatic(TrieTreeNodeStatic* root)
{
	setRoot(root);
}


TrieTreeStatic::~TrieTreeStatic()
{
}

void TrieTreeStatic::setRoot(TrieTreeNodeStatic* newRoot)
{
	root = newRoot;
}

