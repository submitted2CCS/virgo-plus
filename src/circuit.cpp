#include "circuit.h"
#include "../lib/virgo/src/fieldElement.h"
#include <algorithm>
#include "immintrin.h"

layeredCircuit layeredCircuit::readFromStream(char *)
{
    //todo
    //bitlength
    //dag toposort
    //init Vmult, Vadd for each layer
    return layeredCircuit();
}

layeredCircuit layeredCircuit::randomize(int layerNum, int eachLayer)
{
    layeredCircuit c;
    int gateSize = 1 << eachLayer;
    c.circuit.resize(layerNum);
    c.size = layerNum;
    c.circuit[0].bitLength = eachLayer;
    c.circuit[0].size = gateSize;
    c.circuit[0].gates.resize(gateSize);
    for (int i = 0; i < gateSize; ++i)
    {
        c.circuit[0].gates[i] = gate(gateType::Input, 0, random(), 0);
    }
    for (int i = 1; i < layerNum; ++i)
    {
        c.circuit[i].bitLength = eachLayer;
        c.circuit[i].size = gateSize;
        c.circuit[i].gates.resize(gateSize);
        for (int j = 0; j < gateSize; ++j)
        {
            c.circuit[i].gates[j] = gate((rand() & 1) == 0 ? gateType::Add : gateType::Mult, rand() % i, rand() % gateSize, rand() % gateSize);
        }
    }
    return c;
}

void layeredCircuit::subsetInit()
{
    for (int i = 0; i < size; ++i)
    {
        circuit[i].dadBitLength.resize(size);
        circuit[i].dadSize.resize(size);
        circuit[i].dadId.resize(size);
        circuit[i].maxDadBitLength = -1;
        for (int j = 0; j < size; ++j)
        {
            circuit[i].dadBitLength[j] = -1;
        }
    }
    for (int i = size - 1; i > 0; --i)
    {
        std::vector<std::unordered_map<u64, u64>> Vdad;
        std::vector<std::vector<u64>> dadId;
        std::vector<std::pair<u64, int>> dadSort;
        for (int j = 0; j < size; ++j)
        {
            Vdad.push_back(std::unordered_map<u64, u64>());
            dadId.push_back(std::vector<u64>());
        }
        circuit[i].maxDadSize = 0;
        for (u64 j = circuit[i].size - 1; j < circuit[i].size; --j)
        {
            auto g = circuit[i].gates[j];
            u64 l = g.l, v = g.v;
            if (Vdad[l].find(v) == Vdad[l].end())
            {
                Vdad[l][v] = dadId[l].size();
                dadId[l].push_back(v);
                if (circuit[i].dadBitLength[l] == -1 || dadId[l].size() > (1 << circuit[i].dadBitLength[l]))
                {
                    circuit[i].dadBitLength[l]++;
                }
                if (circuit[i].dadBitLength[l] > circuit[i].maxDadBitLength)
                {
                    circuit[i].maxDadBitLength = circuit[i].dadBitLength[l];
                    circuit[i].star = l;
                }
                if (dadId[l].size() > circuit[i].maxDadSize)
                    circuit[i].maxDadSize = dadId[l].size();
            }
            circuit[i].gates[j].lv = Vdad[l][v];
        }

        for (int j = 0; j < i; ++j)
        {
            circuit[i].dadSize[j] = dadId[j].size();
            circuit[i].dadId[j].resize(dadId[j].size());
            for (int k = 0; k < dadId[j].size(); ++k)
                circuit[i].dadId[j][k] = dadId[j][k];
        }
    }
}
