/*
 * main.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: dorav
 */
#include <math.h>
#include <array>
#include <iostream>
#include <ostream>
#include <vector>
#include <cmath>
#include <random>
#include <random>

#include "heap.h"
#include "unlimited.h"
using std::endl;
using std::cout;
using std::vector;

using namespace AlgorithmsMaman14;

template <typename Counters>
ostream& printCounters(ostream& out)
{
	return out << "compare, emplace, copy = "
			   << Counters().getStaticCounters().compareCounter << ", "
			   << Counters().getStaticCounters().emplaceCounter << ", "
			   << Counters().getStaticCounters().copyCounter;
}

class Counters
{
public:
	void resetStaticCounters()
	{
		auto& counters = getStaticCounters();
		counters = Counters();
	}

	Counters& getStaticCounters()
	{
		static Counters c;
		return c;
	}

	int compareCounter = 0;
	int emplaceCounter = 0;
	int copyCounter = 0;
};

template <typename Comperable, typename Counter = Counters>
struct CountInteger
{
	CountInteger() = default;
	CountInteger(Comperable obj)
	: real(obj)
	{
	}

	CountInteger(const CountInteger& other)
	: real(other.real)
	{
		Counter().getStaticCounters().copyCounter++;
	}

	CountInteger(CountInteger&& other)
	: real(std::move(other.real))
	{
		Counter().getStaticCounters().emplaceCounter++;
	}

	CountInteger& operator=(const CountInteger& other)
	{
		Counter().getStaticCounters().emplaceCounter++;
		real = std::move(other.real);
		return *this;
	}

	operator const Comperable&() const { return real; }

	bool operator > (const CountInteger& other) const
	{
		Counter().getStaticCounters().compareCounter++;
		return real > other.real;
	}

	Comperable real;
};

template <typename Heap, typename Counter = Counters>
void printSons(const Heap& heap, std::size_t length)
{
	printCounters<Counter>(cout);
	for (auto i = 0u; i < length; ++i)
	{
		cout << "Heap[" << i << "] = " << heap[i] << endl;
	}
	printCounters<Counter>(cout);
}

typedef DHeap<CountInteger<int>> Heap;

template <typename Heap, typename Counter = Counters>
void assertSorted(const Heap&, std::size_t originalSize, const Heap& heap, std::size_t length)
{
	if (originalSize != length)
		throw std::runtime_error("Sorted array length != original array length");

	for (auto i = 0u; i < length - 1; ++i)
	{
		if (heap[i] > heap[i+1])
			throw std::runtime_error("Array not sorted");
	}
}



int main()
{
	int min = 0;
	int max = 1024;
	std::vector<CountInteger<int>> original;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(min, max);

	for (int i = 0; i < 50; ++i)
	{
		original.push_back(distribution(gen));
		cout << "DORAV - " << original.front() << endl;
	}

	for (int i = 2; i < 7; ++i)
	{
		auto toSort = original;
		Counters().resetStaticCounters();
		heap_sort(i, toSort);
		assertSorted(original, original.size(), toSort, toSort.size());
		cout << "For d = " << i << " ";
		printCounters<Counters>(cout) << endl;
	}
	return 0;
}


