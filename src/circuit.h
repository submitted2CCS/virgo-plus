#pragma once

#include <vector>
#include <unordered_map>
#include <utility>
#include <virgo/polyCommit.hpp>
#include <unordered_set>

enum class gateType
{
	Mult, Add, Input, Zero, Relay, Xor, Naab, Minus, Not, AntiMinus, AntiNaab, SIZE
};

class gate
{
public:
	gateType ty;
	int l;
    u64 u, v, lv;
	gate()
	{
	}
	gate(gateType t, int ll, u64 uu, u64 vv)
	{
		ty = t;
		u = uu;
		v = vv;
		l = ll;
		lv = 0;
	}
};


class layer
{
public:
	vector<gate> gates;
	int bitLength;
	u64 size;

	vector<vector<u64>> dadId;
	vector<int> dadBitLength;
	vector<u64> dadSize;
    u64 maxDadSize;
	int maxDadBitLength, star;

//	vector<int> dadSort;
//    vector<int> dadTh;
};

class layeredCircuit
{
public:
	vector<layer> circuit;
	int size;

	static layeredCircuit readFromStream(char *);
	static layeredCircuit randomize(int layer, int eachLayer);
	void subsetInit();
};

