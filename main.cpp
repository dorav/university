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

#include "heap.h"
#include "unlimited.h"
using std::endl;
using std::cout;
using std::vector;

using namespace AlgorithmsMaman14;

template <typename Counters>
void printCounters(ostream& out)
{
	out << "compare, emplace, copy = "
		<< Counters().getStaticCounters().compareCounter << ", "
		<< Counters().getStaticCounters().emplaceCounter << ", "
		<< Counters().getStaticCounters().copyCounter << endl;
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
void printSons(Heap& heap, std::size_t length)
{
	printCounters<Counter>(cout);
	for (auto i = 0u; i < length; ++i)
	{
		cout << "Heap[" << i << "] = " << heap.storage()[i] << endl;
	}
	printCounters<Counter>(cout);
}

int main()
{
//	CountInteger<int> a[] = new CountInteger<int>[len];
//	a[0] = 2;
//	a[1] = 4;
//	a[2] = 5;
	std::vector<CountInteger<int>> a;
	for (int i = 0; i < 11; ++i)
	{
		a.push_back(i);
	}
	Counters().resetStaticCounters();
//	DHeap<CountInteger<int>, CountInteger<int>*> heap(1, a, len);
	DHeap<CountInteger<int>> heap(1, a);
	int len = heap.length();
//	for (int i = 0; i < 11; ++i)
//	{
//		cout << "parent of " << i << " = " << heap.parentOf(i) << endl;
//	}


	printSons(heap, len);

	cout << "Sorting ---------------------------" << endl;
	heap.sort();

	printSons(heap, len);
	return 0;
}


