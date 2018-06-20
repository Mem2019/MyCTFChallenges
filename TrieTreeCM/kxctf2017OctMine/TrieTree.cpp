#include "stdafx.h"
#include "TrieTree.h"

TrieTree::TrieTree()
{
	root = new TrieTreeNode();
}



TrieTree::~TrieTree()
{
	delete root;
}



void TrieTree::addString(const TrieTreeString& str)
{
	insertString((TrieTreeNode*)root, str);
	//root is definitely RadixTreeNode
}



void TrieTree::insertString(TrieTreeNode* node, const TrieTreeString& str)
//insert str into children of node(ignore content of node)
{
	TrieTreeChildren<TrieTreeNode> children = node->getChildren();

	size_t nCommonPrefix = 0;
	TrieTreeNode* child = nullptr;
	for (TrieTreeNode* iChild : children)
	{
		child = iChild;
		TrieTreeString strChild = child->getContent();
		nCommonPrefix = numOfCommonPrefix(strChild, str);
		if (nCommonPrefix != 0)// must >0
			break;
	}
	/*
	now child is the child that has the same prefix as str if prefix is not 0,
	the last one in vector if prefix is 0
	*/
	if (nCommonPrefix == 0)
	{
		node->addChild(str, 1);
	}
	else
	{
		size_t childLen = child->getContent().size();
		size_t strLen = str.size();
		if (nCommonPrefix == childLen && nCommonPrefix == strLen)
		{
			child->setNumber(child->getNumber() + 1);
		}
		else if (nCommonPrefix == childLen && nCommonPrefix < strLen)
		{
			TrieTreeString subString = str.substr(nCommonPrefix, strLen - nCommonPrefix);
			insertString(child, subString);
		}
		else if (nCommonPrefix < childLen && nCommonPrefix == strLen)
		{
			TrieTreeString suffix = child->getContent().substr(
				nCommonPrefix, childLen - nCommonPrefix);
			TrieTreeChildren<TrieTreeNode> childSaved = child->getChildren();
			int numSaved = child->getNumber();

			child->clearChildren_NoDelete();
			child->setNumber(1);
			child->setContent(str);
			child->addChild(suffix, numSaved);
			//e.g. the child is "abc123", we add "abc"
			//so the child node now become "abc", and the "123" added below it
			//"123" nodes has all the children and number saved

			TrieTreeNode* newNode = child->getChildren()[0];
			newNode->setChildren(childSaved);
		}
		else if (nCommonPrefix < childLen && nCommonPrefix < strLen)
		{
			TrieTreeString commonPrefix = str.substr(0, nCommonPrefix);
			TrieTreeString strSuffix = str.substr(nCommonPrefix, strLen - nCommonPrefix);
			TrieTreeString childSuffix = child->getContent().substr(
				nCommonPrefix, childLen - nCommonPrefix);
			//extract prefix and suffix

			TrieTreeChildren<TrieTreeNode> childSaved = child->getChildren();
			int numSaved = child->getNumber();
			//save number and children

			child->clearChildren_NoDelete();
			child->setNumber(0);
			child->setContent(commonPrefix);
			child->addChild(childSuffix, numSaved);
			child->addChild(strSuffix, 1);
			//e.g. the child is "abc123", we add "abc456"
			//so the child node now become "abc", and the "123","456" added below it
			//"123" node has all the children and number saved
			//"456" node has num 1 and no child

			TrieTreeNode* newNode = child->getChildren()[0];
			newNode->setChildren(childSaved);
		}
		else
		{
			//impossible
			::ExitProcess(-1);
		}
	}
}

size_t TrieTree::numOfCommonPrefix(const TrieTreeString& str1, const TrieTreeString& str2)
{
	int ret = 0;
	size_t len1 = str1.size();
	size_t len2 = str2.size();
	size_t min = len1 > len2 ? len2 : len1;
	for (size_t i = 0; i < min; i++)
	{
		if (str1[i] == str2[i])
			ret++;
		else
			break;
	}
	return ret;
}

