/*
 * unlimited.cc
 *
 *  Created on: Oct 28, 2014
 *      Author: dor
 */
#include "unlimited.h"
#include <exception>
#include <iterator>
#include <iostream>
#include <stdlib.h>

#include <iosfwd>
#include <limits>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <iomanip>

using namespace std;

static const int BASE = 10000;

struct IndexOutOfRangeException : public std::exception {};

int& Unlimited::LeadingZerosVector::operator [](unsigned int i)
{
	if (i < size())
		return vector::operator [](i);
	if (i == size())
	{
		push_back(0);
		return vector::operator [](i);
	}
	throw IndexOutOfRangeException();
}

int Unlimited::LeadingZerosVector::operator [](unsigned int i) const
{
	if (i < size())
		return vector::operator [](i);
	else
		return 0;
}

bool Unlimited::operator ==(const string& str) const
{
	return str == (string)*this;
}

void Unlimited::printLastDigit(stringstream& out) const
{
	out << setw(0);
	out << digits.back();
}

void Unlimited::printWithFillers(stringstream& out) const
{
	out << setw(log10(BASE)) << setfill('0');
	copy(++digits.rbegin(),digits.rend(), ostream_iterator<int>(out, ""));
}

Unlimited::operator string() const
{
	if (digits.empty())
		return "0";

	stringstream out;
	if (isNegative)
		out << "-";

	printLastDigit(out);
	printWithFillers(out);

	return out.str();
}

void Unlimited::parseString(const string& value)
{
	auto leastSignificantDigit = value.rend();
	auto mostSignificantDigit = value.rbegin();

	isNegative = value[0] == '-';
	if (isNegative)
		--leastSignificantDigit;

	parseNonNegativeStringRepresentation(mostSignificantDigit, leastSignificantDigit);
}

string Unlimited::extractNextDigits(const string::const_reverse_iterator& leastSignificant,
									string::const_reverse_iterator& mostSignificant)
{
	string currentDigits;
	for (int i = 0; i < log10(BASE); ++i)
	{
		if (mostSignificant == leastSignificant)
			break;

		currentDigits += *mostSignificant;
		++mostSignificant;
	}
	return string(currentDigits.rbegin(), currentDigits.rend());
}

int stoi(const std::string& str)
{
	return strtol(str.c_str(), NULL, 0);
}

void Unlimited::parseNonNegativeStringRepresentation(string::const_reverse_iterator mostSignificant,
													 string::const_reverse_iterator leastSignificant)
{
	for(; mostSignificant != leastSignificant;)
	{
		string currentDigits = extractNextDigits(leastSignificant, mostSignificant);
		insertMostSignificantDigit(stoi(currentDigits));
	}
}

Unlimited::Unlimited(const string& value)
{
	parseString(value);
}

void Unlimited::insertMostSignificantDigit(int digit)
{
	digits.push_back(digit);
}

struct DigitsSum
{
	DigitsSum(int first, int sec, int _carry)
	: sum(first + sec + _carry)
	, carry(sum / BASE)
	{
		if (carry != 0)
			sum %= BASE;
	}

	int sum;
	int carry;
};

Unlimited Unlimited::operator++(int)
{
	Unlimited old(*this);
	operator++();
	return old;
}

Unlimited& Unlimited::operator++()
{
	operator+=(string("1"));
	return *this;
}

Unlimited::OrderedCouple Unlimited::orderByAbsoluteValue(const Unlimited& first, const Unlimited& other)
{
	if (first.isAbsolutlyBigger(other))
		return OrderedCouple(other, first);
	return OrderedCouple(first, other);
}

void Unlimited::differentSignAddition(const Unlimited& other)
{
	Unlimited::OrderedCouple args = orderByAbsoluteValue(*this, other);

	args.bigger.subtractBy(args.smaller.digits, digits);
	isNegative = args.bigger.isNegative;
}

void Unlimited::operator+=(const Unlimited& other)
{
	if (isSameSign(other))
		addDigitsFrom(other);
	else
		differentSignAddition(other);
}

bool Unlimited::isAbsolutlyBigger(const Unlimited& other) const
{
	if (digits.size() > other.digits.size())
		return true;

	for (unsigned int i = 0; i < digits.size(); ++i)
	{
		if (digits[i] > other.digits[i])
			return true;
	}

	return false;
}

bool Unlimited::operator>(const Unlimited& other) const
{
	if (isSameSign(other))
		return isAbsolutlyBigger(other) == isNegative;

	if (isNegative)
		return false;

	return true;
}

void Unlimited::operator-=(const Unlimited& other)
{
	if (isSameSign(other))
		sameSignSubtraction(other);
	else
		addDigitsFrom(other);
}

void Unlimited::sameSignSubtraction(const Unlimited& other)
{
	OrderedCouple args = orderByAbsoluteValue(*this, other);

	args.bigger.subtractBy(args.smaller.digits, digits);

	if (&args.smaller == this)
		isNegative = !isNegative;
}

bool Unlimited::isSameSign(const Unlimited& other) const
{
	return other.isNegative == isNegative;
}

Unlimited Unlimited::operator+(const Unlimited& other) const
{
	Unlimited result(other);
	result += *this;
	return result;
}

void Unlimited::addDigitsFrom(const Unlimited& other)
{
	int longerLength = std::max(digits.size(), other.digits.size());

	DigitsSum currentSum(0, 0, 0);

	for (int i = 0; i < longerLength; ++i)
	{
		currentSum = DigitsSum(other.digits[i], digits[i], currentSum.carry);
		digits[i] = currentSum.sum;
	}

	if (currentSum.carry != 0)
		insertMostSignificantDigit(currentSum.carry);
}

int Unlimited::borrowAmount(bool shouldBorrow) const
{
	return shouldBorrow;
}

struct DigitDifference
{
	DigitDifference(int first, int sec, bool shouldBorrow)
	: difference(first - sec - borrowAmount(shouldBorrow))
	, haveBorrowed(difference < 0)
	{
		if (haveBorrowed)
			difference += BASE;
	}

	int difference;
	bool haveBorrowed;

private:
	int borrowAmount(bool shouldBorrow) const
	{
		return shouldBorrow;
	}
};

Unlimited Unlimited::operator-(const Unlimited& other) const
{
	Unlimited result(*this);
	result -= other;
	return result;
}

void Unlimited::subtractBy(const LZVector& other, LZVector& result) const
{
	DigitDifference currentDifference(0, 0, 0);

	for (unsigned int i = 0; i < digits.size(); ++i)
	{
		currentDifference = DigitDifference(digits[i], other[i], currentDifference.haveBorrowed);
		result[i] = currentDifference.difference;
	}

	removeLeadingZeros(result);
}

bool Unlimited::hasLeadingZero(LZVector& result)
{
	return result.back() == 0;
}

void Unlimited::removeLeadingZeros(LZVector& result)
{
	while (result.empty() == false && hasLeadingZero(result))
		result.pop_back();
}
ostream& operator<<(ostream& out, const Unlimited& number)
{
	return out << (string)number;
}

Unlimited::Unlimited(const Unlimited& other)
: digits(other.digits)
, isNegative(other.isNegative)
{
}

void Unlimited::swap(Unlimited& first, Unlimited& second)
{
	std::swap(first.digits, second.digits);
	std::swap(first.isNegative, second.isNegative);
}

void Unlimited::operator =(Unlimited&& other)
{
	swap(*this, other);
}

Unlimited::Unlimited(Unlimited&& other)
: digits(std::move(other.digits))
, isNegative(other.isNegative)
{
}
