#pragma once
#include "ItemList.h"
#include <memory>

class Node
{
public:

	// Constructor and destructor
	Node(int id);
	~Node();

	// Getters
	int id() { return _id; }
	ItemList &itemList() { return _itemList; }
	const ItemList &itemList() const { return _itemList; }
	int GetCurrentMoney() const { return money; }
	void SetCurrentMoney(int amount) { money += amount; }

private:

	int _id; /**< Id of this node. */
	ItemList _itemList; /**< All items owned by this node. */
	int money = 300;
};

using NodePtr = std::shared_ptr<Node>;
