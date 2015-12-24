/*
 * main.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: dorav
 */
#include <math.h>
#include <stdlib.h>
#include <algorithm>
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

using namespace std::chrono;
using std::endl;
using std::cout;
using std::vector;

using namespace AlgorithmsMaman14;

const int DHEAP_MAX = 5;
const int DHEAP_MIN = 2;

// Helper method for printing the statistics
template <typename Counters>
ostream& printCounters(ostream& out, int d, int numberOfRuns)
{
	return out << "compare, emplace, copy = "
			   << Counters::getOverallCounters(d).compareCounter / numberOfRuns << ", "
			   << Counters::getOverallCounters(d).emplaceCounter / numberOfRuns << ", "
			   << Counters::getOverallCounters(d).copyCounter / numberOfRuns << " - "
			   << "took " << (Counters::getOverallCounters(d).timeToSort.count() / numberOfRuns ) << "us";
}

// Helper class for measuring statistics
class Counters
{
public:
	// Used to collect data over time, the index can be used to keep track of several statistics
	// In this case, index will be the 'd' parameter of the DHeap
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

	// Used to combine data collected after a single event into the total
	void addStaticCounters(int index, microseconds timeToSort)
	{
		auto& counters = getStaticCounters();
		auto& overall = getOverallCounters(index);
		overall.compareCounter += counters.compareCounter;
		overall.emplaceCounter += counters.emplaceCounter;
		overall.copyCounter += counters.copyCounter;
		overall.timeToSort += timeToSort;

		counters.reset();
	}

	// Use this to increase counters of a specific event and then use reset
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

	CountInteger& operator=(CountInteger&& other)
	{
		Counters::getStaticCounters().emplaceCounter++;
		real = std::move(other.real);
		return *this;
	}

	operator const Comperable&() const { return real; }

	bool operator>(const CountInteger& other) const
	{
		Counters::getStaticCounters().compareCounter++;
		return real > other.real;
	}

	Comperable real;
};

typedef DHeap<CountInteger<int>> Heap;

template <typename Heap>
void printHeap(ostream& out, const Heap& heap, std::size_t length)
{
	out << "{ ";
	for (auto i = 0u; i < length; ++i)
		out << "{ " << i << " : " << heap[i] << " }, ";
	out << " }";
}

template <typename Heap, typename Counter = Counters>
void assertSorted(std::size_t originalSize, const Heap& heap, std::size_t length)
{
	if (originalSize != length)
		throw std::runtime_error("Sorted array length != original array length");

	if (length < 2)
		return;

	for (unsigned int i = 0; i < length - 1; ++i)
	{
		if (heap[i] > heap[i + 1])
		{
			cerr << "Following array is not sorted: ";
			printHeap(cerr, heap, length);
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
	for (int d = DHEAP_MIN; d <= DHEAP_MAX; ++d)
	{
		auto toSort = original; // copying the original string, to work with same input every time.
		trackSortWith(d, toSort); // sorting with a specific DHeap while tracking the number of compares, copies and emplacements.
		assertSorted(original.size(), toSort, toSort.size()); // making sure the array is indeed sorted.
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
		original.emplace_back(randomize());

	sortWithDifferentDHeaps(original);
}

void printSortStatistics(int arraySizeToSort, int d, int reps)
{
	cout << "Sorting " << arraySizeToSort << " elements with d = " << d << " ";
	printCounters<Counters>(cout, d, reps) << endl;
}

void collectStatistics(int arraySizeToSort, int reps)
{
	for (int d = DHEAP_MIN; d <= DHEAP_MAX; ++d)
	{
		printSortStatistics(arraySizeToSort, d, reps);
		Counters::getOverallCounters(d).reset();
	}
}

void repeatSorting(int arraySizeToSort, int reps)
{
	for (int i = 0; i < reps; ++i)
		sortNElements(arraySizeToSort);
}

void measureDHeapSorts(int arraySizeToSort)
{
	int reps = 100;
	repeatSorting(arraySizeToSort, 100);
	collectStatistics(arraySizeToSort, reps);
}

int main()
{
	measureDHeapSorts(50);
	measureDHeapSorts(100);
	measureDHeapSorts(200);

	return 0;
}


