#include "TrieTreeAbs.h"


TrieTreeAbs::TrieTreeAbs()
{
}


TrieTreeAbs::~TrieTreeAbs()
{
}

bool TrieTreeAbs::operator==(const TrieTreeAbs& that) const
{
	return *this->root == *that.root;
}

