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

class DFA
{
public:
	void PrintStates() const;

	DFA(const NFA &);
	DFA(const DFA &);
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
	bool is_element(size_t) const;
	size_t trans(size_t) const;
private:
	bool accepting;
	size_t state_num;
	const std::vector<bool> characteristic_set;
	State *transitions[a_size];
};

int main()
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

DFA::DFA(const DFA &dfa)
{
	std::vector<bool> characteristic_0(dfa.states.size(), false);
	std::vector<bool> characteristic_1(dfa.states.size(), false);
	for (size_t i = 0; i < dfa.states.size(); i++)
	{
		if (dfa.states[i]->is_accepting())
			characteristic_1[i] = true;
		else
			characteristic_0[i] = true;
	}
	if (characteristic_0[0] == true)
	{
		states.push_back(new State(characteristic_0, false));
		states.push_back(new State(characteristic_1, true));
	}
	else
	{
		states.push_back(new State(characteristic_1, true));
		states.push_back(new State(characteristic_0, false));
	}

	struct transitions
	{
		transitions(size_t from, size_t to) : from(from), to(to), checked(false) {}
		size_t from, to;
		bool checked;
	};

	bool valid = false;
	while (!valid)
	{
		valid = true;
		for (size_t i = 0; i < a_size; i++)
		{
			size_t size = states.size();
			for (size_t j = 0; j < size; j++)
			{
				std::vector<transitions> trans;
				trans.reserve(dfa.states.size());
				for (size_t k = 0; k < dfa.states.size(); k++)
					if (states[j]->is_element(k))
						trans.push_back({ k, dfa.states[k]->trans(i) });
				for (size_t k = 0; k < trans.size(); k++)
					for (size_t l = 0; l < states.size(); l++)
					{
						if (trans[k].to == 0)
							break;
						if (states[l]->is_element(trans[k].to - 1))
						{
							trans[k].to = l + 1;
							break;
						}
					}
				State *temp = nullptr;
				for (size_t k = 0; k < trans.size(); k++)
					if (!trans[k].checked)
					{
						std::vector<bool> characteristic(dfa.states.size(), false);
						size_t current = trans[k].to;
						for (size_t l = k; l < trans.size(); l++)
							if (trans[l].to == current)
							{
								characteristic[trans[l].from] = true;
								trans[l].checked = true;
							}
						if (k == 0)
							temp = new State(characteristic, states[j]->is_accepting());
						else
							states.push_back(new State(characteristic, states[j]->is_accepting()));
					}
				if (*states[j] == *temp)
					delete temp;
				else
				{
					delete states[j];
					states[j] = temp;
					valid = false;
				}
			}
		}
	}
	for (size_t i = 0; i < states.size(); i++)
	{
		states[i]->assign_num(i);
		for (size_t j = 0; j < a_size; j++)
			for (size_t k = 0;; k++)
				if (states[i]->is_element(k))
				{
					size_t to = dfa.states[k]->trans(j);
					if (to-- != 0)
						for (size_t l = 0;; l++)
							if (states[l]->is_element(to))
							{
								states[i]->attach(states[l], alphabet[j]);
								break;
							}
					break;
				}
	}
}
bool DFA::State::is_accepting() const
{
	return accepting;
}
bool DFA::State::is_element(size_t element) const
{
	return characteristic_set[element];
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