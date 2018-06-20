#include "TrieTreeNode.h"
#include "TrieTreeChildren.h"

TrieTreeNode::TrieTreeNode():TrieTreeNodeAbs(){}

TrieTreeNode::TrieTreeNode(const TrieTreeString& content, int number):
	TrieTreeNodeAbs(content, number){}

TrieTreeNode::~TrieTreeNode()
{
	freeAllChildren();
}

void TrieTreeNode::freeAllChildren()
{
	for (TrieTreeNodeAbs* cs : children)//must be RadixTreeNode
	{
		delete cs;//virtual destructor
	}
}



void TrieTreeNode::addChild(const TrieTreeString& content, int number)
{
	children.push_back(new TrieTreeNode(content, number));
}



void TrieTreeNode::clearChildren_NoDelete()
//this function will NOT delete the children!!! 
//but only clear the vector
{
	children.clear();
}

void TrieTreeNode::setChildren(const TrieTreeChildren<TrieTreeNode>& newChildren)
{
	for (TrieTreeNodeAbs* child : newChildren)
	{
		children.push_back(child);
	}
}

TrieTreeChildren<TrieTreeNode> TrieTreeNode::getChildren()
{
	TrieTreeChildren<> childrenAbs = TrieTreeNodeAbs::getChildren();
	TrieTreeChildren<TrieTreeNode> children;
	for (TrieTreeNodeAbs* childAbs : childrenAbs)
	{
		children.push_back(dynamic_cast<TrieTreeNode*>(childAbs));
		//pre: all of the children must be RadixTreeNode* type
		//which must be true for a RadixTreeNode class
	}
	return children;
}




