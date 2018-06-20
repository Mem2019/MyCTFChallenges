#pragma once
#include <iostream>
class TrieTreeNodeAbs;
template<typename Node = TrieTreeNodeAbs, size_t MAX_NUM_CHILD = 32>
class TrieTreeChildren
{
public:
	TrieTreeChildren() 
	{
		this->num = 0;
	}
	void push_back(Node* child)
	{
		if (num >= MAX_NUM_CHILD)
		{
			std::cerr << "No more trie tree nodes can be added!" << std::endl;
			exit(-1);
		}
		children[num] = child;
		num++;
	}
	Node* const * begin() const
	{
		return &children[0];
	}
	Node* const * end() const
	{
		return &children[num];
	}
	void clear()
	{
		this->num = 0;
	}
	size_t size() const
	{
		return num;
	}
	Node* operator[](size_t idx) const
	{
		if (idx >= num)
		{
			std::cerr << "No no no, array out of bound when accessing trie tree child node" << std::endl;
			exit(-1);
		}
		return children[idx];
	}
private:
	Node* children[MAX_NUM_CHILD];
	size_t num;
};

