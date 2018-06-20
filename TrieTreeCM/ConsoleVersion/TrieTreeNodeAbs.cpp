#include "TrieTreeNodeAbs.h"


TrieTreeNodeAbs::TrieTreeNodeAbs() : content()
{
	this->number = 0;
}

TrieTreeNodeAbs::TrieTreeNodeAbs(const TrieTreeString& content, int number)
{

	this->content = content;
	this->number = number;
}

int TrieTreeNodeAbs::getNumber()
{
	return number;
}

void TrieTreeNodeAbs::setContent(const TrieTreeString& newContent)
{
	content = newContent;
}


void TrieTreeNodeAbs::setNumber(int num)
{
	number = num;
}


TrieTreeString TrieTreeNodeAbs::getContent()
{
	return content;
}

TrieTreeChildren<> TrieTreeNodeAbs::getChildren()
{
	return children;//clone a new vector children
}

bool TrieTreeNodeAbs::operator!=(const TrieTreeNodeAbs& that) const
{
	return !(*this == that);
}
TrieTreeNodeAbs::~TrieTreeNodeAbs()
{
}


bool TrieTreeNodeAbs::operator==(const TrieTreeNodeAbs& that) const
{
	if (this->content != that.content)
		return false;
	if (this->children.size() != that.children.size())
		return false;
	if (this->number != that.number)
		return false;
	for (TrieTreeNodeAbs* child : children)
	{
		TrieTreeNodeAbs* node =
			that.findChildGivenContent(child->getContent());
		if (node == nullptr)
			return false;
		if (*node != *child)
			return false;
	}
	return true;
}

TrieTreeNodeAbs* TrieTreeNodeAbs::findChildGivenContent(const TrieTreeString& str) const
//given a string, find the corresponding RadixTreeNode*
//if there is no corresponding one, return null
{
	for (TrieTreeNodeAbs* child : children)
	{
		if (child->getContent() == str)
		{
			return child;
		}
	}
	return nullptr;
}
