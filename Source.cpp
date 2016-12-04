#include <iostream>
#include <fstream>
#include <vector>

constexpr size_t a_size = 2;
constexpr char alphabet[a_size + 1] = "ab";
constexpr size_t a_index(char c)
{
	return c - 'a';
}
constexpr bool in_alphabet(char c)
{
	/*return (c >= 'a' && c <= 'z') ? true :
		(c >= 'A' && c <= 'Z') ? true : false; */
	return (c == 'a' || c == 'b') ? true : false;
}

class NFA
{
public:
	NFA(char);
	NFA &Complete();
	std::vector<bool> &Closure(std::vector<bool> &) const;
	std::vector<bool> Move(const std::vector<bool> &, char) const;
	static NFA Concatenate(const NFA &lhs, const NFA &rhs);
	static NFA Or(const NFA &lhs, const NFA &rhs);
	static NFA Star(const NFA &arg);
	static NFA Plus(const NFA &arg);
	size_t size() const;
	size_t accepting() const;
private:
	class State;
	NFA() {};
	void closure_recursion(size_t, size_t, std::vector<bool> &) const;
	char exit_char;
	State *exit_state;
	std::vector<State *> states;
};
class NFA::State
{
public:
	State(bool accepting = false) : accepting(accepting) {}
	void attach(char, State *);
	void assign_num(size_t);
	std::vector<size_t> trans_list(char c) const;
	bool is_accepting() const;
private:
	bool accepting;
	size_t state_num;
	struct Transition
	{
		Transition(char c, State *state) : c(c), state(state) {}
		const char c;
		State *const state;
	};
	std::vector<Transition> transitions;
};

class DFA
{
public:
	DFA(const NFA &);
	DFA(const DFA &);

	void PrintStates() const;
	void PrintHeaders(std::ostream &) const;
	void PrintDefinitions(std::ostream &) const;
private:
	struct StateInfo
	{
		bool accepting;
		size_t transitions[a_size];
	};
	std::vector<StateInfo> state_info;

	static bool is_nonempty(const std::vector<bool> &);
};

int main(int argc, char *argv[])
{
	system("pause");
	try 
	{
		//NFA nfa = NFA::Concatenate(NFA::Concatenate(NFA::Concatenate(NFA::Star(NFA('a')), NFA::Or(NFA('a'), NFA('b'))), NFA('a')), NFA('a')).Complete();
		NFA nfa = NFA::Star(NFA::Concatenate(NFA::Or(NFA('a'), NFA('b')), NFA::Or(NFA('a'), NFA::Concatenate(NFA('b'), NFA('b'))))).Complete();
		DFA dfa(nfa);
		DFA optimal(dfa);
		dfa.PrintStates();
		optimal.PrintStates();

		std::ofstream out(argc < 2 ? "out.cpp" : argv[1]);

		optimal.PrintHeaders(out);
		out << std::endl;
		optimal.PrintDefinitions(out);
		out.close();
	}
	catch (char *msg)
	{
		std::cout << msg << std::endl;
	}
	system("pause");
}

NFA::NFA(char c) : exit_char(c)
{
	if (in_alphabet(c) || c == '\0')
	{
		exit_state = new State();
		states.push_back(exit_state);
	}
	else
		throw "NFA::NFA: Character is not in alphabet!";
}
NFA &NFA::Complete()
{
	State *end = new State(true);
	exit_state->attach(exit_char, end);
	states.push_back(end);
	for (size_t i = 0; i < states.size(); i++)
		(*states[i]).assign_num(i);
	return *this;
}
std::vector<bool> &NFA::Closure(std::vector<bool> &subset) const
{
	for (size_t i = 0; i < subset.size(); i++)
		if (subset[i])
			closure_recursion(i, i, subset);
	return subset;
}
std::vector<bool> NFA::Move(const std::vector<bool> &subset, char c) const
{
	std::vector<bool> result(states.size(), false);
	for (size_t i = 0; i < subset.size(); i++)
	{
		if (subset[i])
		{
			std::vector<size_t> trans = states[i]->trans_list(c);
			for (size_t j = 0; j < trans.size(); j++)
				result[trans[j]] = true;
		}
	}
	return Closure(result);
}
void NFA::closure_recursion(size_t current, size_t checked, std::vector<bool> &subset) const
{
	if (current > checked)
		subset[current] = true;
	else if (subset[current] == false || checked == current) // note that this fails if there is an epsilon loop
	{
		subset[current] = true;
		std::vector<size_t> trans = states[current]->trans_list('\0');
		for (size_t i = 0; i < trans.size(); i++)
			closure_recursion(trans[i], checked, subset);
	}
}
NFA NFA::Concatenate(const NFA &lhs, const NFA &rhs)
{
	lhs.exit_state->attach(lhs.exit_char, rhs.states[0]);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 1);
	for (size_t i = 0; i < lhs.states.size(); i++)
		result.states.push_back(lhs.states[i]);
	for (size_t i = 0; i < rhs.states.size(); i++)
		result.states.push_back(rhs.states[i]);
	result.exit_char = rhs.exit_char;
	result.exit_state = rhs.exit_state;
	return result;
}
NFA NFA::Or(const NFA &lhs, const NFA &rhs)
{
	State *in = new State(), *out = new State();
	in->attach('\0', lhs.states[0]);
	in->attach('\0', rhs.states[0]);
	lhs.exit_state->attach(lhs.exit_char, out);
	rhs.exit_state->attach(rhs.exit_char, out);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 3);
	result.states.push_back(in);
	for (size_t i = 0; i < lhs.states.size(); i++)
		result.states.push_back(lhs.states[i]);
	for (size_t i = 0; i < rhs.states.size(); i++)
		result.states.push_back(rhs.states[i]);
	result.states.push_back(out);
	result.exit_char = '\0';
	result.exit_state = out;
	return result;
}
NFA NFA::Star(const NFA &arg)
{
	State *hub = new State();
	hub->attach('\0', arg.states[0]);
	arg.exit_state->attach(arg.exit_char, hub);
	NFA result;
	result.states.reserve(arg.states.size() + 2);
	result.states.push_back(hub);
	for (size_t i = 0; i < arg.states.size(); i++)
		result.states.push_back(arg.states[i]);
	result.exit_char = '\0';
	result.exit_state = hub;
	return result;

}
NFA NFA::Plus(const NFA &arg)
{
	State *in = new State(), *out = new State();
	in->attach('\0', arg.states[0]);
	arg.exit_state->attach(arg.exit_char, out);
	out->attach('\0', in);
	NFA result;
	result.states.reserve(arg.states.size() + 3);
	result.states.push_back(in);
	for (size_t i = 0; i < arg.states.size(); i++)
		result.states.push_back(arg.states[i]);
	result.states.push_back(out);
	result.exit_char = '\0';
	result.exit_state = out;
	return result;
}
size_t NFA::size() const
{
	return states.size();
}
size_t NFA::accepting() const
{
	for (size_t i = 0; i < states.size(); i++)
		if (states[i]->is_accepting())
			return i;
}
void NFA::State::attach(char c, NFA::State *to)
{
	transitions.push_back({ c, to });
}
void NFA::State::assign_num(size_t num)
{
	state_num = num;
}
std::vector<size_t> NFA::State::trans_list(char c) const
{
	std::vector<size_t> result;
	result.reserve(transitions.size());
	for (size_t i = 0; i < transitions.size(); i++)
		if (transitions[i].c == c)
			result.push_back(transitions[i].state->state_num);
	return result;
}
bool NFA::State::is_accepting() const
{
	return accepting;
}

DFA::DFA(const NFA &nfa)
{
	std::vector<std::vector<bool>> states;
	std::vector<bool> state_set(nfa.size(), false);
	state_set[0] = true;
	states.push_back(nfa.Closure(state_set));
	state_info.push_back(StateInfo());
	if (state_set[nfa.accepting()] == true)
		state_info[0].accepting = true;
	for (size_t state_index = 0; state_index < states.size(); state_index++)
	{
		for (size_t char_index = 0; char_index < a_size; char_index++)
		{
			state_set = nfa.Move(states[state_index], alphabet[char_index]);
			if (is_nonempty(state_set))
			{
				for (size_t prev_state_index = 0;; prev_state_index++)
				{
					if (prev_state_index == states.size())
					{
						states.push_back(state_set);
						state_info.push_back(StateInfo());
						if (state_set[nfa.accepting()] == true)
							state_info[prev_state_index].accepting = true;
						state_info[state_index].transitions[char_index] = prev_state_index + 1;
						break;
					}
					if (state_set == states[prev_state_index])
					{
						state_info[state_index].transitions[char_index] = prev_state_index + 1;
						break;
					}
				}
			}
			else
				state_info[state_index].transitions[char_index] = 0;
		}
	}
}
DFA::DFA(const DFA &dfa)
{
	struct transition
	{
		transition(size_t from_old, size_t from_new, size_t to) : from_old(from_old), from_new(from_new), to(to), marked(false) {}
		size_t from_old, from_new, to;
		bool marked;
	};
	std::vector<size_t> states;
	states.reserve(dfa.state_info.size());
	size_t num_states = 2;
	if (dfa.state_info[0].accepting)
		for (size_t index = 0; index < dfa.state_info.size(); index++)
			states.push_back(dfa.state_info[index].accepting ? 1 : 2);
	else
		for (size_t index = 0; index < dfa.state_info.size(); index++)
			states.push_back(dfa.state_info[index].accepting ? 2 : 1);
	bool consistent = false;
	while (!consistent)
	{
		consistent = true;
		for (size_t char_index = 0; char_index < a_size; char_index++)
		{
			std::vector<transition> transitions;
			transitions.reserve(states.size());
			std::vector<size_t> current_trans_vals(num_states);
			for (size_t dfa_state = states.size(); dfa_state-- > 0;)
			{
				size_t trans = (dfa.state_info[dfa_state].transitions[char_index] == 0) ?
					0 : states[dfa.state_info[dfa_state].transitions[char_index] - 1];
				current_trans_vals[states[dfa_state] - 1] = trans;
				transitions.push_back({ dfa_state, states[dfa_state], trans });
			}
			for (size_t i = transitions.size(); i-- > 0;)
			{
				if (transitions[i].marked == false)
				{
					if (transitions[i].to != current_trans_vals[transitions[i].from_new - 1])
					{
						consistent = false;
						states[transitions[i].from_old] = ++num_states;
						current_trans_vals[transitions[i].from_new - 1] = num_states;
					}
					for (size_t j = i; j-- > 0;)
					{
						if ((transitions[j].marked == false) && (transitions[j].from_new == transitions[i].from_new) &&
							(transitions[j].to == current_trans_vals[transitions[j].from_new - 1]))
						{
							transitions[j].marked = true;
							states[transitions[j].from_old] = states[transitions[i].from_old];
						}
					}
				}
			}
		}
	}
	state_info = std::vector<StateInfo>(num_states);
	for (size_t i = 0; i < states.size(); i++)
	{
		state_info[states[i] - 1].accepting = dfa.state_info[i].accepting;
		for (size_t j = 0; j < a_size; j++)
			state_info[states[i] - 1].transitions[j] = (dfa.state_info[i].transitions[j] == 0) ? 0 : states[dfa.state_info[i].transitions[j] - 1];
	}
}
bool DFA::is_nonempty(const std::vector<bool> &subset)
{
	for (std::vector<bool>::const_iterator it = subset.cbegin(); it != subset.end(); it++)
		if (*it == true)
			return true;
	return false;
}

void DFA::PrintStates() const
{
	for (size_t i = 0; i < state_info.size(); i++)
	{
		std::cout << "State " << i + 1 << ": " << (state_info[i].accepting ? "Accepting\n" : "\n");
		for (size_t j = 0; j < a_size; j++)
			if (state_info[i].transitions[j] != 0)
				std::cout << "\tMove(" << i + 1 << ", " << alphabet[j] << ") = " << state_info[i].transitions[j] << std::endl;
	}
}
void DFA::PrintHeaders(std::ostream &out) const
{
	for (size_t i = 0; i < state_info.size(); )
		out << "bool State_" << ++i << "(std::string::const_iterator it, std::string::const_iterator end);" << std::endl;
}
void DFA::PrintDefinitions(std::ostream &out) const
{
	for (size_t i = 0; i < state_info.size(); i++)
	{
		out << "bool State_" << i + 1 << "(std::string::const_iterator it, std::string::const_iterator end)\n";
		out << "{\n";
		out << "\tif (it != end)\n";
		out << "\t{\n";
		out << "\t\tswitch (*it++)\n";
		out << "\t\t{\n";
		for (size_t j = 0; j < a_size; j++)
		{
			if (state_info[i].transitions[j] != 0)
			{
				out << "\t\tcase '" << alphabet[j] << "':\n";
				out << "\t\t\treturn State_" << state_info[i].transitions[j] << "(it, end);\n";
			}
		}
		out << "\t\tdefault :\n";
		out << "\t\t\treturn false;\n";
		out << "\t\t}\n";
		out << "\t}\n";
		out << "\telse\n";
		out << "\t\treturn " << (state_info[i].accepting ? "true;\n" : "false;\n");
		out << "}" << std::endl;
	}
}