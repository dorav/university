/*
 * heap.h
 *
 *  Created on: Dec 21, 2015
 *      Author: dorav
 */

#ifndef HEAP_H_
#define HEAP_H_
#include <iostream>

#include "heap_storage.h"

using namespace std;
namespace AlgorithmsMaman14{

template <typename T, typename RandomAccessStorage = std::vector<T>>
class DHeap
{
public:
	/* The design allows placing specialized datastructures as storage.
	 * Some of them requires different parameters for construction.
	 * (e.g c-style array requires ArrayData)
	 *
	 * In order to know which parameters to pass, take a look at the DHeapData class
	 */
	template <typename... Args>
	DHeap(std::size_t numberOfSons_, Args&&... args)
	: numberOfSons(numberOfSons_)
	, data(std::forward<Args>(args)...)
	{
		// The API supports receiving a storage and converting it into a heap.
		build_max_heap();
	}

	/* This constructor is used to build a heap onto the DataStructure without
	 * using the entire DataStructure.
	 */
	template <typename... Args>
	DHeap(std::size_t numberOfSons_, std::size_t heapSize, Args&&... args)
	: numberOfSons(numberOfSons_)
	, data(heapSize, std::forward<Args>(args)...)
	{
		// The API supports receiving a storage and converting it into a heap.
		build_max_heap();
	}

	struct HeapIsEmptyException : public std::runtime_error { HeapIsEmptyException() : std::runtime_error("root method called on empty heap"){} };

	// Allows accessing the heap's root element.
	const T& root() const
	{
		if (isEmpty())
			throw HeapIsEmptyException();

		return *data[0];
	}

	/* Pushes a copy of obj into the heap
	 * Proof of concept to support priority queue
	 */
	void push(const T& obj)
	{
		data.push(obj);
		std::swap(*data[0], *data[data.length()]);
		max_heapify(0);
	}

	bool isEmpty() const
	{
		return data.length() <= 0;
	}

	/*  Provides access to the heap as a random access storage.
	 *  Needs to be used with care and with conjunction with the length() method.
	 */
	const RandomAccessStorage& storage() const
	{
		return data.data;
	}

	std::size_t length() const
	{
		return data.heapSize;
	}

	// Rebuilds and validates the object as a heap after construction or a call to sort()
	void build_max_heap()
	{
		// If the heap has one element or less, it is already a valid heap
		if (length() < 2)
			return;

		auto maxIndex = length() - 1;

		// Fixing the heap - node after node up the tree. Stopping when reaching the root;
		for (std::size_t i = parentOf(maxIndex); i > 0; --i)
			max_heapify(i);

		// Fixing the root.
		max_heapify(0);
	}

	/* NOTE: After calling this method the heap will decrease it's size to one.
	 * (This is the smallest number of elements that represents a valid heap).
	 * The stored data can be accessed via the storage() method.
	 * The length of the sorted array is the number returned by the function.
	 */
	std::size_t sort()
	{
		auto originalLen = length();
		if (length() > 1) // Does not need to do anything if one or zero elements.
			sortImpl();

		return originalLen;
	}


protected:
	// Corrects the heap property, returns whether the heap was changed or not
	void max_heapify(std::size_t parent)
	{
		std::size_t largest = parent;

		// Looping all the parent's sons, finding the one with the largest value
		for (auto child = beginChild(parent); child != endChild(); ++child)
			if (*child > *data[largest])
				largest = child.index();

		// Correcting the heap, swapping the parent with the biggest node
		if (largest != parent)
		{
			std::swap(*data[largest], *data[parent]);
			// a lower heap might have been broken, needs to get fixed
			max_heapify(largest);
		}
	}

public:
	// Returns the parent of a given index in heap representation of an array.
	// Root is the parent of itself
	std::size_t parentOf(std::size_t index) const
	{
		if (index == 0)
			return 0;

		return (index -1) / numberOfSons;
	}

	struct ChildEndIterator {};

	// This class is used to iterate over the sons of a given node in a heap representation of the data.
	class ChildIterator
	{
	public:
		ChildIterator(std::size_t parent_, std::size_t length_, DHeap<T, RandomAccessStorage>& heap_)
		: parent(parent_)
		, length(length_)
		, heap(heap_)
		, current({0, childOf(0), NULL})
		{
			setValue();
		}

		std::size_t childOf(std::size_t sonNumber) const
		{
			return parent * heap.numberOfSons + sonNumber + 1;
		}

		void operator++()
		{
			nextChild();
			setValue();
		}

		const T& operator*()
		{
			return *current.value;
		}

		bool operator!=(const ChildEndIterator&)
		{
			return isValid();
		}

		std::size_t index()
		{
			return current.index;
		}

	private:
		struct Child
		{
			std::size_t number;
			std::size_t index;
			const T* value;
		};

		std::size_t parent;
		std::size_t length;
		DHeap<T, RandomAccessStorage>& heap;
		Child current;

		void setValue()
		{
			if (isValid())
				current.value = heap.data[current.index];
		}

		bool isValid()
		{
			return current.index < length && current.number < heap.numberOfSons;
		}

		void nextChild()
		{
			++current.number;
			current.index = childOf(current.number);
		}
	};

	ChildIterator beginChild(std::size_t parent)
	{
		return ChildIterator(parent, data.length(), *this);
	}

	ChildIterator beginChild(std::size_t parent, std::size_t maxIndex)
	{
		return ChildIterator(parent, maxIndex, *this);
	}

	ChildEndIterator endChild()
	{
		return ChildEndIterator();
	}

protected:
	std::size_t numberOfSons;
	DHeapData<T, RandomAccessStorage> data;

public:
	void sortImpl()
	{
		// Sorting is based on the heap sort algorithm.
		// The heap property is kept in the range [0.. i]
		// While the range [i..length() -1] gets sorted values.
		for (std::size_t i = data.length() - 1; i >= 1; --i)
		{
			// Removing the biggest value from the heap at range [0 .. i - 1]
			std::swap(*data[0], *data[i]);
			data.heapSize--;
			max_heapify(0);
		}
	}
};

template <typename T>
void heap_sort(std::size_t numberOfSons, std::vector<T>& storage)
{
	// We want to edit the given data, using the array version of the heap for that.
	ArrayData<T> array(&storage[0], storage.size());
	DHeap<T, T*> heap(numberOfSons, array);
//	DHeap<T> heap(numberOfSons, storage);
	heap.sort();
}

}



#endif /* HEAP_H_ */
