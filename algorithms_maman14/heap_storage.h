#ifndef HEAP_ALGS
#define HEAP_ALGS
#include <iostream>
#include <stdexcept>

namespace AlgorithmsMaman14{

// Helper class for storing metadata about the array storage for the heap
template <typename T>
struct ArrayData
{
	ArrayData(T* array_, std::size_t length)
	: array(array_)
	, arrayLength(length)
	, heapSize(length)
	{
	}

	ArrayData(T* array_, std::size_t length, std::size_t heapSize_)
	: array(array_)
	, arrayLength(length)
	, heapSize(heapSize_)
	{
	}

	T* array;
	std::size_t arrayLength;
	std::size_t heapSize; // needs explicit initialization
};


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
 * DHeap depends upon some of the data-members and the functions
 * in this class. If you specialize it, you must write them too.
 */
template <typename T, typename RandomAccessStorage>
class DHeapData
{
public:
	DHeapData()
	: heapSize(0)
	{
	}

	// Explicit as this is a heavy copy constructor
	explicit DHeapData(const RandomAccessStorage& data_)
	: data(data_)
	, heapSize(data.size())
	{
	}

	// Explicit as this is a heavy copy constructor
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
	DHeapData(ArrayData<T> array)
	: data(array.array)
	, heapSize(array.heapSize)
	, arraySize(array.heapSize)
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

	struct HeapIsFullException : public std::runtime_error { HeapIsFullException() : std::runtime_error("push into a full heap"){} };

	void push(const T& obj)
	{
		if (heapSize >= arraySize)
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
