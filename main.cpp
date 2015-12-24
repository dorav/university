/*
 * main.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: dorav
 */
#include <math.h>
#include <stdlib.h>
#include <array>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <random>
#include <unordered_map>
#include <chrono>
#include <sstream>

#include "heap.h"
#include "unlimited.h"

using namespace std::chrono;
using std::endl;
using std::cout;
using std::vector;

using namespace AlgorithmsMaman14;

template <typename Counters>
ostream& printCounters(ostream& out, int d, int numberOfRuns)
{
	return out << "compare, emplace, copy = "
			   << Counters::getOverallCounters(d).compareCounter / numberOfRuns << ", "
			   << Counters::getOverallCounters(d).emplaceCounter / numberOfRuns << ", "
			   << Counters::getOverallCounters(d).copyCounter / numberOfRuns << " - "
			   << "took " << (Counters::getOverallCounters(d).timeToSort.count() / numberOfRuns ) << "us";
}

class Counters
{
public:
	static Counters& getOverallCounters(int index)
	{
		static std::unordered_map<int, Counters> overall;
		if (overall.find(index) == overall.end())
			overall.emplace(index, Counters());
		return overall.at(index);
	}

	void reset()
	{
		*this = Counters();
	}

	void addStaticCounters(int index, microseconds timeToSord)
	{
		auto& counters = getStaticCounters();
		auto& overall = getOverallCounters(index);
		overall.compareCounter += counters.compareCounter;
		overall.emplaceCounter += counters.emplaceCounter;
		overall.copyCounter += counters.copyCounter;
		overall.timeToSort += timeToSord;

		counters.reset();
	}

	static Counters& getStaticCounters()
	{
		static Counters c;
		return c;
	}

	int compareCounter = 0;
	int emplaceCounter = 0;
	int copyCounter = 0;
	std::chrono::microseconds timeToSort;
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
		Counters::getStaticCounters().copyCounter++;
	}

	CountInteger(CountInteger&& other)
	: real(std::move(other.real))
	{
		Counters::getStaticCounters().emplaceCounter++;
	}

	CountInteger& operator=(const CountInteger& other)
	{
		Counters::getStaticCounters().emplaceCounter++;
		real = std::move(other.real);
		return *this;
	}

	operator const Comperable&() const { return real; }

	bool operator > (const CountInteger& other) const
	{
		Counters::getStaticCounters().compareCounter++;
		return real > other.real;
	}

	Comperable real;
};

template <typename Heap, typename Counter = Counters>
void printSons(const Heap& heap, std::size_t length)
{
	for (auto i = 0u; i < length; ++i)
	{
		cout << "Heap[" << i << "] = " << heap[i] << endl;
	}
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
		{
			for (unsigned int i = 0; i < heap.size(); ++i)
				cout << "[" << i << "]" << heap[i] << endl;
			stringstream err; err << "[" << i << "] = " << heap[i] << " <= " << heap[i+1] << " = [" << i + 1 << "]";
			throw std::runtime_error("Array not sorted" + err.str());
		}
	}
}

template <typename T>
microseconds timedSort(int heapSons, T& toSort)
{
	auto start = steady_clock::now();
	heap_sort(heapSons, toSort);
	auto end = steady_clock::now();

	return duration_cast<microseconds> (end - start);
}

template <typename T>
void trackSortWith(std::size_t d, T& toSort)
{
	Counters().getStaticCounters().reset();
	auto timeToSort = timedSort(d, toSort);
	Counters().addStaticCounters(d, timeToSort);
}

template <typename T>
void sortWithDifferentDHeaps(const T& original)
{
	for (int d = 2; d < 7; ++d)
	{
		auto toSort = original;
		trackSortWith(d, toSort);
		assertSorted(original, original.size(), toSort, toSort.size());
	}
}

int randomize()
{
	int min = 0;
	int max = 1024;
	static std::mt19937 generator(time(NULL));
	static std::uniform_int_distribution<> distribution (min, max);

	return distribution(generator);
}

void sortNElements(int arraySizeToSort)
{
	std::vector<CountInteger<int> > original;

	for (int i = 0; i < arraySizeToSort; ++i)
		original.push_back(randomize());

	sortWithDifferentDHeaps(original);
}

void measureDHeapSorts(int arraySizeToSort)
{
	for (int repetition = 0; repetition < 10; ++repetition)
		sortNElements(arraySizeToSort);

	for (int d = 2; d < 7; ++d)
	{
		cout << "Sorting " << arraySizeToSort << " elements with d = " << d << " ";
		printCounters<Counters>(cout, d, 2000) << endl;
		Counters::getOverallCounters(d).reset();
	}
}

int main()
{
	std::mt19937 generator(time(NULL));

	measureDHeapSorts(50);
	measureDHeapSorts(100);
	measureDHeapSorts(200);

	return 0;
}


