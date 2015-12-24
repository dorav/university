#ifndef HEAP_ALGS
#define HEAP_ALGS
#include <iostream>
#include <stdexcept>

namespace AlgorithmsMaman14{

template <typename T, typename RandomAccessStorage>
class DHeapData
{
/*
 * This represents an underlying data holder for the Dheap class.
 * It is required to have random access operator enabled.
 *
 * Note: This is meant to provide supports "out of the box" for std::vectors
 * 		 and act as an interface for any other random-access storage such as
 * 		 std::arrays and c-style arrays.
 * 		 Specialization may be required to support other types.
 *
 * Usage outside of the DHeap implementation is discouraged.
 */
public:
	DHeapData()
	: heapSize(0)
	{
	}

	explicit DHeapData(const RandomAccessStorage& data_)
	: data(data_)
	, heapSize(data.size())
	{
	}

	explicit DHeapData(std::size_t heapSize_, const RandomAccessStorage& data_)
	: data(data_)
	, heapSize(heapSize_)
	{
	}

	DHeapData(RandomAccessStorage&& data_)
	: data(std::forward<RandomAccessStorage>(data_))
	, heapSize(data.size())
	{
	}

	DHeapData(std::size_t heapSize_, RandomAccessStorage&& data_)
	: data(std::forward<RandomAccessStorage>(data_))
	, heapSize(heapSize_)
	{
	}

	const T* operator[] (std::size_t location) const
	{
		return &data[location];
	}

	T* operator[] (std::size_t location)
	{
		return &data[location];
	}

	size_t length() const
	{
		return heapSize;
	}

	void push(const T& obj)
	{
		data.push_back(obj);
		heapSize = data.size();
	}

	RandomAccessStorage data;
	std::size_t heapSize;
};

/*
 * Template specialization of the above class to support
 * c-style arrays.
 */
template <typename T>
class DHeapData<T, T*>
{
public:
	DHeapData(T* data_, std::size_t length_)
	: data(data_)
	, heapSize(length_)
	, arraySize(length_)
	{
	}

	DHeapData(T* data_, std::size_t length_, std::size_t heapSize_)
	: data(data_)
	, heapSize(heapSize_)
	, arraySize(length_)
	{
	}

	const T* operator[] (std::size_t location) const
	{
		return &data[location];
	}

	T* operator[] (std::size_t location)
	{
		return &data[location];
	}

	size_t length() const
	{
		return heapSize;
	}

	class HeapIsFullException : public std::runtime_error { HeapIsFullException() : std::runtime_error("push into a full heap"){} };

	void push(const T& obj)
	{
		if (heapSize + 1 >= arraySize)
			throw HeapIsFullException();

		++heapSize;
		*this->operator [](heapSize) = obj;
	}

	const T* storage() const
	{
		return data;
	}

	T* data;
	std::size_t heapSize;

protected:
	std::size_t arraySize;
};

}

#endif
