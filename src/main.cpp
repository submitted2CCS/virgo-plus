#include <cstdio>
#include <string>
#include <fstream>
#include <regex>
#include <map>
#include <vector>
#include <queue>
#include <cassert>
#include <cstdlib>
#include "verifier.h"
using namespace std;

string source_line;
int tgt;
long long src0, src1;

regex add_gate("P V[0-9]+ = V[0-9]+ \\+ V[0-9]+ E");
regex mult_gate("P V[0-9]+ = V[0-9]+ \\* V[0-9]+ E");
regex constant_assign_gate("P V[0-9]+ = [\\-]*[0-9]+ E");
regex input_gate("P V[0-9]+ = I[0-9]+ E");
regex output_gate("P O[0-9]+ = V[0-9]+ E");
regex pass_gate("P V[0-9]+ = V[0-9]+ PASS V[0-9]+ E");
regex xor_gate("P V[0-9]+ = V[0-9]+ XOR V[0-9]+ E");
regex minus_gate("P V[0-9]+ = V[0-9]+ minus V[0-9]+ E");
regex naab_gate("P V[0-9]+ = V[0-9]+ NAAB V[0-9]+ E");
regex not_gate("P V[0-9]+ = V[0-9]+ NOT V[0-9]+ E");
regex exp_sum_gate("P V[0-9]+ = V[0-9]+ EXPSUM V[0-9]+ E");
regex bit_test_gate("P V[0-9]+ = V[0-9]+ BIT V[0-9]+ E");

smatch base_match;

enum gate_types
{
	add = 0,
	mult = 1,
	dummy = 2,
	input = 3,
	not_gate_id = 6,
	minus_gate_id = 7,
	xor_gate_id = 8,
	naab_gate_id = 9,
	output_gate_id = 11,
	relay_gate_id = 10,
	exp_sum_gate_id = 12,
	bit_test_gate_id = 13
};

class mid_gate
{
public:
	gate_types ty;
	int g;
	long long u, v;
	mid_gate() = default;
	mid_gate(gate_types TY, int gg, int uu, int vv)
	{
		ty = TY;
		g = gg;
		u = uu;
		v = vv;
	}
};

class mid_layer
{
public:
	vector<mid_gate> gates;
	int bit_len;
	bool is_parallel;
	int block_size;
	int log_block_size;
};

class mid_layered_circuit
{
public:
	vector<mid_layer> layers;
	int depth;
};

class DAG_gate
{
public:
	pair<int, long long> input0, input1, id;
	int layered_id;
	int layered_lvl;
	bool is_output;
	gate_types ty;
	vector<pair<int, long long> > outputs;
};

class DAG_circuit
{
public:
	map<pair<int, long long>, DAG_gate> circuit;
};

mid_layered_circuit sha256;
DAG_circuit sha256_dag, sha256_dag_copy;

int relay_counter = 0;

void add_relay(pair<int, long long> id0, pair<int, long long> id1, int left_or_right)
{
	DAG_gate &g0 = sha256_dag_copy.circuit[id0];
	DAG_gate &g1 = sha256_dag_copy.circuit[id1];
	if (g0.layered_lvl == g1.layered_lvl + 1)
		return;
	else
	{
		auto pre_id = g1.id;
		for (int i = g1.layered_lvl + 1; i < g0.layered_lvl; ++i)
		{
			DAG_gate g;
			g.id = make_pair((int)'R', relay_counter++);
			g.layered_lvl = i;
			g.is_output = false;
			g.ty = relay_gate_id;
			g.input0 = pre_id;
			g.input1 = make_pair('N', 0);
			sha256_dag.circuit[g.id] = g;
			pre_id = g.id;
		}
		if (left_or_right == 0)
		{
			sha256_dag.circuit[g0.id].input0 = pre_id;
		}
		else
		{
			sha256_dag.circuit[g0.id].input1 = pre_id;
		}
	}
}

int repeat;
layeredCircuit c;

void DAG_to_layered()
{
	vector<int> layer_gate_count;
	map<pair<int, long long>, int> in_deg;
	for (auto &x : sha256_dag.circuit)
	{
		auto &id = x.first;
		auto &g = x.second;
		for (auto y : g.outputs)
		{
			in_deg[y]++;
		}
		g.layered_lvl = -1;
	}
	queue<pair<int, long long> > q;
	for (auto & x : sha256_dag.circuit)
	{
		auto &id = x.first;
		if (in_deg[id] == 0)
		{
			assert((x.second).ty == input);
			(x.second).layered_lvl = 0;
			q.push(id);
		}

	}
	int max_lvl = -1;
	while (!q.empty())
	{
		auto cur = q.front();
		q.pop();
		auto &g = sha256_dag.circuit[cur];
		max_lvl = max(max_lvl, g.layered_lvl);
		for (auto x : g.outputs)
		{
			in_deg[x]--;
			sha256_dag.circuit[x].layered_lvl = max(sha256_dag.circuit[x].layered_lvl, g.layered_lvl + 1);
			if (in_deg[x] == 0)
			{
				q.push(x);
			}
		}
	}
	fprintf(stdout, "sha256_dag.circuit.size: %d\n", sha256_dag.circuit.size());
	layer_gate_count.resize(max_lvl + 1);
	for (int i = 0; i <= max_lvl; ++i)
		layer_gate_count[i] = 0;
	for (auto & x : sha256_dag.circuit)
	{
		auto &id = x.first;
		auto &g = x.second;
		layer_gate_count[g.layered_lvl]++;
	}

	sha256.depth = max_lvl + 1;
	sha256.layers.resize(sha256.depth);
	for (int i = 0; i <= max_lvl; ++i)
	{
		int lgc = layer_gate_count[i];
		int full_layer_count = 1;
		int bit_length = 0;
		while (lgc)
		{
			full_layer_count *= 2;
			lgc /= 2;
			bit_length++;
		}
		sha256.layers[i].bit_len = bit_length;
	}

	vector<int> layer_counter;
	layer_counter.resize(max_lvl + 1);
	for (int i = 0; i <= max_lvl; ++i)
	{
		layer_counter[i] = 0;
	}
	for (auto & x : sha256_dag.circuit)
	{
		auto &id = x.first;
		auto &g = x.second;
		assert(g.layered_lvl  >= 0 && g.layered_lvl < 20);
		g.layered_id = layer_counter[g.layered_lvl]++;
	}
	int gatesCnt = 0;
	c.size = max_lvl + 1;
	c.circuit.resize(max_lvl + 1);
	sha256_dag_copy = sha256_dag;
	for (int i = 0; i <= max_lvl; ++i)
	{
		c.circuit[i].size = layer_counter[i] * repeat;
		c.circuit[i].bitLength = 0;
		while (c.circuit[i].size > (1 << c.circuit[i].bitLength))
			++c.circuit[i].bitLength;
		c.circuit[i].gates.resize(c.circuit[i].size);
		int t = -1;
		for (auto & x : sha256_dag.circuit)
		{
			auto &id = x.first;
			auto &g = x.second;
			if (g.layered_lvl != i)
				continue;
			++t;
			int ll, uu, vv;
			if (g.ty == 3)
			{
				ll = 0;
				uu = g.input0.second;
				vv = 0;
			}
			else if (sha256_dag_copy.circuit[g.input0].layered_lvl == i - 1)
			{
				ll = sha256_dag_copy.circuit[g.input1].layered_lvl;
				uu = sha256_dag_copy.circuit[g.input0].layered_id;
				vv = sha256_dag_copy.circuit[g.input1].layered_id;
			}
			else
			{
				ll = sha256_dag_copy.circuit[g.input0].layered_lvl;
				uu = sha256_dag_copy.circuit[g.input1].layered_id;
				vv = sha256_dag_copy.circuit[g.input0].layered_id;
			}
			gateType type;
			switch ((int)g.ty)
			{
			case 0:
				type = gateType::Add;
				break;
			case 1:
				type = gateType::Mult;
				break;
			case 3:
				type = gateType::Input;
				break;
			case 6:
				type = gateType::Not;
				ll = i - 1;
				break;
			case 7:  // minus not commutative
				type = sha256_dag_copy.circuit[g.input0].layered_lvl == i - 1 ? gateType::Minus : gateType::AntiMinus;
				break;
			case 8:
				type = gateType::Xor;
				break;
			case 9:  // naab not commutative
				type = sha256_dag_copy.circuit[g.input0].layered_lvl == i - 1 ? gateType::Naab : gateType::AntiNaab;
				break;
			default:
				fprintf(stderr, "wrong gate %d\n", g.ty);
				assert(false);
			}
			for (int j = 0; j < repeat; ++j)
				if (type == gateType::Input)
					c.circuit[i].gates[j * layer_counter[i] + t] = gate(type, ll, uu, vv);
				else
					c.circuit[i].gates[j * layer_counter[i] + t] = gate(type, ll, j * layer_counter[i - 1] + uu, j * layer_counter[ll] + vv);
			++gatesCnt;
		}
	}
	fprintf(stderr, "all gates: %d\n", gatesCnt);
}

int input_count = 0;

void read_circuit(ifstream &circuit_in)
{
	while (getline(circuit_in, source_line))
	{
		if (std::regex_match(source_line, base_match, add_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld + V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = add;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, mult_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld * V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = mult;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, constant_assign_gate))
		{
			assert(false);
			sscanf(source_line.c_str(), "P V%d = %lld E", &tgt, &src0);
		}
		else if (std::regex_match(source_line, base_match, input_gate))
		{
			sscanf(source_line.c_str(), "P V%d = I%lld E", &tgt, &src0);
			DAG_gate g;
			g.is_output = false;
			input_count++;
			g.ty = input;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'S', random());
			g.input1 = make_pair((int)'N', 0);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
		}
		else if (std::regex_match(source_line, base_match, output_gate))
		{
			sscanf(source_line.c_str(), "P O%d = V%lld E", &tgt, &src0);
			sha256_dag.circuit[make_pair((int)'V', (int)src0)].is_output = true;
		}
		else if (std::regex_match(source_line, base_match, xor_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld XOR V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = xor_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, naab_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld NAAB V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = naab_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, minus_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld minus V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = minus_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, not_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld NOT V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = not_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else if (std::regex_match(source_line, base_match, exp_sum_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld EXPSUM V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = exp_sum_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			for (u64 i = g.input0.second; i <= g.input1.second; ++i)
			{
				sha256_dag.circuit[make_pair((int)'V', i)].outputs.push_back(g.id);
			}
		}
		else if (std::regex_match(source_line, base_match, bit_test_gate))
		{
			sscanf(source_line.c_str(), "P V%d = V%lld BIT V%lld E", &tgt, &src0, &src1);
			DAG_gate g;
			g.is_output = false;
			g.ty = bit_test_gate_id;
			g.id = make_pair((int)'V', (int)tgt);
			g.input0 = make_pair((int)'V', (int)src0);
			g.input1 = make_pair((int)'V', (int)src1);
			sha256_dag.circuit[make_pair((int)'V', tgt)] = g;
			sha256_dag.circuit[g.input0].outputs.push_back(g.id);
			sha256_dag.circuit[g.input1].outputs.push_back(g.id);
		}
		else
		{
			assert(false);
		}
	}

	DAG_to_layered();
}


int main(int argc, char **argv)
{
	repeat = atoi(argv[2]);
	F::init();
	ifstream circuit_in(argv[1]);
	read_circuit(circuit_in);
	circuit_in.close();

	c.subsetInit();
    prover p(c);
    verifier v(&p, c);
    v.verify();
	fprintf(stdout, "mult counter %d, add counter %d\n", F::multCounter, F::addCounter);
    return 0;
}
