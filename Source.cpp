#include <iostream>
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
		(c >= 'A' && c <= 'Z') ? true : false;*/
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

class DFA_Alt;

class DFA
{
	friend DFA_Alt;
public:
	void PrintStates() const;

	DFA(const NFA &);
private:
	class State;
	bool is_nonempty(const std::vector<bool> &) const;
	std::vector<State *> states;
};
class DFA::State
{
public:
	void print_description() const;

	State(const std::vector<bool> &, bool = false);
	const std::vector<bool> &Characteristic() const;
	void attach(State *, char);
	bool operator==(const State &) const;
	void mark_accepting(size_t);
	void assign_num(size_t);
	bool is_accepting() const;
	size_t trans(size_t) const;
private:
	bool accepting;
	size_t state_num;
	const std::vector<bool> characteristic_set;
	State *transitions[a_size];
};


class DFA_Alt
{
public:
	DFA_Alt(const NFA &);
	DFA_Alt(const DFA &);

	void PrintStates() const;
private:
	struct StateInfo
	{
		bool accepting;
		size_t transitions[a_size];
	};
	std::vector<StateInfo> state_info;
};

DFA_Alt::DFA_Alt(const NFA &nfa)
{

}

DFA_Alt::DFA_Alt(const DFA &dfa)
{
	struct transition
	{
		transition(size_t from_old, size_t from_new, size_t to) : from_old(from_old), from_new(from_new), to(to), marked(false) {}
		size_t from_old, from_new, to;
		bool marked;
	};
	std::vector<size_t> states;
	states.reserve(dfa.states.size());
	size_t num_states = 2;
	if (dfa.states[0]->is_accepting())
		for (std::vector<DFA::State *>::const_iterator it = dfa.states.begin(); it != dfa.states.end(); it++)
			states.push_back((*it)->is_accepting() ? 1 : 2);
	else
		for (std::vector<DFA::State *>::const_iterator it = dfa.states.begin(); it != dfa.states.end(); it++)
			states.push_back((*it)->is_accepting() ? 2 : 1);
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
				size_t trans = (dfa.states[dfa_state]->trans(char_index) == 0) ? 0 : states[dfa.states[dfa_state]->trans(char_index) - 1];
				current_trans_vals[states[dfa_state] - 1] = trans; // sloppy
				transitions.push_back({ dfa_state, states[dfa_state], trans });
			}
			for (size_t i = transitions.size(); i-- > 0;)
				if (transitions[i].marked == false)
				{
					if (transitions[i].to != current_trans_vals[transitions[i].from_new - 1])
					{
						consistent = false;
						states[transitions[i].from_old] = ++num_states;
						current_trans_vals[transitions[i].from_new - 1] = num_states;
					}
					for (size_t j = i; j-- > 0;)
						if ((transitions[j].marked == false) && (transitions[j].from_new == transitions[i].from_new) && (transitions[j].to == current_trans_vals[transitions[j].from_new - 1]))
						{
							transitions[j].marked = true;
							states[transitions[j].from_old] = states[transitions[i].from_old];
						}
				}
		}
	}

	state_info = std::vector<StateInfo>(num_states);
	for (size_t i = 0; i < states.size(); i++)
	{
		state_info[states[i] - 1].accepting = dfa.states[i]->is_accepting();
		for (size_t j = 0; j < a_size; j++)
			state_info[states[i] - 1].transitions[j] = (dfa.states[i]->trans(j) == 0) ? 0 : states[dfa.states[i]->trans(j) - 1];
	}
}
void DFA_Alt::PrintStates() const
{
	for (size_t i = 0; i < state_info.size(); i++)
	{
		std::cout << "State " << i + 1 << ": " << (state_info[i].accepting ? "Accepting\n" : "\n");
		for (size_t j = 0; j < a_size; j++)
			if (state_info[i].transitions[j] != 0)
				std::cout << "\tMove(" << i + 1 << ", " << alphabet[j] << ") = " << state_info[i].transitions[j] << std::endl;
	}
}

int main()
{
	system("pause");
	try 
	{
		NFA nfa = NFA::Concatenate(NFA::Concatenate(NFA::Concatenate(NFA::Star(NFA('a')), NFA::Or(NFA('a'), NFA('b'))), NFA('a')), NFA('a')).Complete();
		//NFA nfa = NFA::Star(NFA::Concatenate(NFA::Or(NFA('a'), NFA('b')), NFA::Or(NFA('a'), NFA::Concatenate(NFA('b'), NFA('b'))))).Complete();
		DFA dfa(nfa);
		DFA_Alt optimal(dfa);
		dfa.PrintStates();
		optimal.PrintStates();
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
	std::vector<bool> state_set(nfa.size(), false);
	state_set[0] = true;
	states.push_back(new State(nfa.Closure(state_set)));
	for (size_t i = 0; i < states.size(); i++)
	{
		for (size_t j = 0; j < a_size; j++)
		{
			state_set = nfa.Move(states[i]->Characteristic(), alphabet[j]);
			if (is_nonempty(state_set))
			{
				State *move_result = new State(state_set);
				size_t k;
				for (k = 0; k < states.size(); k++)
				{
					if (*move_result == *states[k])
					{
						states[i]->attach(states[k], alphabet[j]);
						break;
					}
				}
				if (k == states.size())
				{
					states.push_back(move_result);
					states[i]->attach(states[states.size() - 1], alphabet[j]);
				}
			}
		}
	}
	size_t accepting = nfa.accepting();
	for (size_t i = 0; i < states.size(); i++)
	{
		states[i]->assign_num(i);
		states[i]->mark_accepting(accepting);
	}
}
bool DFA::is_nonempty(const std::vector<bool> &subset) const
{
	for (size_t i = 0; i < subset.size(); i++)
		if (subset[i])
			return true;
	return false;
}
DFA::State::State(const std::vector<bool> &characteristic_set, bool accepting) : characteristic_set(characteristic_set), accepting(accepting)
{
	for (size_t i = 0; i < a_size; i++)
		transitions[i] = nullptr;
}
const std::vector<bool> &DFA::State::Characteristic() const
{
	return characteristic_set;
}
void DFA::State::attach(DFA::State *to, char c)
{
	transitions[a_index(c)] = to;
}
bool DFA::State::operator==(const DFA::State &rhs) const
{
	for (size_t i = 0; i < characteristic_set.size(); i++)
		if (characteristic_set[i] != rhs.characteristic_set[i])
			return false;
	return true;
}
void DFA::State::mark_accepting(size_t nfa_accepting)
{
	if (characteristic_set[nfa_accepting])
		accepting = true;
}
void DFA::State::assign_num(size_t num)
{
	state_num = num;
}

bool DFA::State::is_accepting() const
{
	return accepting;
}
size_t DFA::State::trans(size_t index) const
{
	if (transitions[index] == nullptr)
		return 0;
	return transitions[index]->state_num + 1;
}

void DFA::PrintStates() const
{
	for (size_t i = 0; i < states.size(); i++)
		states[i]->print_description();
}
void DFA::State::print_description() const
{
	std::cout << "State " << state_num << ": " << (accepting ? "Accepting\n" : "\n");
	for (size_t i = 0; i < a_size; i++)
		if (transitions[i] != nullptr)
			std::cout << "\tMove(" << state_num << ", " << alphabet[i] << ") = " << transitions[i]->state_num << std::endl;
}