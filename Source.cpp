#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <cctype>

using std::move;
using std::vector;

#define EPSILON		0

class Node;
typedef std::unique_ptr<Node> pNode;
// produces ugly code for input (cba)*(a|b)

// char index 0 is reserved for epsilon transition

std::string ToUpper(const std::string &src);

class NFA
{
public:
	NFA() = default;
	NFA(char c);
	NFA(const NFA &) = delete;
	NFA(NFA &&mov) = default;
	NFA &operator=(const NFA &) = delete;
	NFA &operator=(NFA &&rhs) = default;
	vector<bool> &Closure(vector<bool> &subset) const;
	vector<bool> Move(const vector<bool> &subset, size_t cIndex) const;
	static NFA Complete(NFA &&arg, size_t acceptingType);
	static NFA Concatenate(NFA &&lhs, NFA &&rhs);
	static NFA Or(NFA &&lhs, NFA &&rhs);
	static NFA Star(NFA &&arg);
	static NFA Plus(NFA &&arg);
	static NFA Merge(vector<NFA> &&nfas);

	size_t Size() const;
	size_t Accepting(const vector<bool> &subset) const;

	static char Alphabet(size_t index) { return alphabet[index]; }
	static size_t AlphabetSize() { return alphabet.size(); }
private:
	class State;
	typedef std::unique_ptr<State> pState;
	void closureRecursion(size_t current, size_t checked, vector<bool> &subset) const;
	size_t exitCIndex;							// all references to exit_char need to be readjusted
	State *exitState;
	vector<pState> states;

	static size_t charIndex(char c);
	static vector<char> alphabet;
};
vector<char> NFA::alphabet(1, '\0');
class NFA::State
{
public:
	State(size_t accepting = 0) : accepting(accepting) {}
	void attach(size_t cIndex, State *to);
	void assignNum(size_t num);
	vector<size_t> transList(size_t cIndex) const;
	size_t AcceptingType() const;
private:
	size_t accepting;
	size_t stateNum;
	struct Transition
	{
		const size_t cIndex;
		State *const state;
	};
	vector<Transition> transitions;
};

class DFA
{
public:
	DFA(const NFA &nfa);
	DFA(const DFA &dfa);

	void PrintStates() const;
	void PrintHeader(std::ostream &out) const;
	void PrintDefinitions(std::ostream &out) const;

	static vector<std::string> Types;									/// figure out a better way of doing this
private:
	static bool isNonempty(const std::vector<bool> &subset);

	struct StateInfo
	{
		size_t accepting = 0;
		vector<size_t> transitions = vector<size_t>(NFA::AlphabetSize(), EPSILON);
	};
	vector<StateInfo> stateInfo;
};
vector<std::string> DFA::Types = vector<std::string>();

class Iterator
{
public:
	Iterator(const std::string::const_iterator &it) : it(it) {}
	Iterator(const Iterator &it) = default;
	~Iterator() = default;
	Iterator &operator=(const Iterator &rhs) = default;
	Iterator &operator++();
	Iterator operator++(int);
	bool operator==(const Iterator &rhs) const { return it == rhs.it; }
	bool operator!=(const Iterator &rhs) const { return it != rhs.it; }
	char operator*() const;
	bool IsChar() const;
	char C() const;
private:
	std::string::const_iterator it;
};

class Node
{
public:
	virtual NFA GenNfa(NFA &&nfa = NFA()) const = 0;
};
class Tree
{
public:
	Tree(const std::string &input);
	NFA GenNfa(size_t acceptingType) const { return NFA::Complete(node->GenNfa(), acceptingType); }
private:
	pNode node;
};
class Terminal : public Node
{
public:
	Terminal(char symbol) : symbol(symbol) {}
	NFA GenNfa(NFA &&nfa = NFA()) const { return NFA(symbol); }
private:
	const char symbol;
};
class NonTerminal : public Node
{
protected:
	vector<pNode> nodes;
};
class Q : public NonTerminal
{
public:
	Q(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class R : public NonTerminal
{
public:
	R(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class S : public NonTerminal
{
public:
	S(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class T : public NonTerminal
{
public:
	T(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class U : public NonTerminal
{
public:
	U(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class V : public NonTerminal
{
public:
	V(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class W : public NonTerminal
{
public:
	W(Iterator &it, Iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};

int main(int argc, char *argv[])
{
	//std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	try 
	{
		vector<NFA> nfas;
		for (size_t i = 1;; i++)
		{
			std::string expression;
			std::cout << "Regular Expression: ";
			std::cin >> expression;
			if (expression == "$")
				break;
			DFA::Types.push_back(move(expression));
			std::cout << "\t-> ";
			std::cin >> expression;
			Tree syntaxTree(expression);
			nfas.push_back(syntaxTree.GenNfa(i));
		}
		DFA dfa = DFA(DFA(NFA::Merge(move(nfas))));
		dfa.PrintStates();
		std::ofstream out(argc < 2 ? "out.cpp" : argv[1]);
		dfa.PrintHeader(out);
		dfa.PrintDefinitions(out);

		
		/*
		optimal.PrintHeaders(out);
		out << std::endl;
		optimal.PrintDefinitions(out);
		out.close();*/
	}
	catch (char *msg)
	{
		std::cout << msg << std::endl;
	}
	//std::chrono::time_point<std::chrono::high_resolution_clock> t = std::chrono::high_resolution_clock::now();
	//std::cout << "\nExecution time: " << std::chrono::duration_cast<std::chrono::microseconds>(t - t0).count() << std::endl;
	system("pause");
}

NFA::NFA(char c) : exitCIndex(charIndex(c))
{
	states.emplace_back(new State);
	exitState = states.back().get();
}
NFA NFA::Complete(NFA &&arg, size_t acceptingType)
{
	arg.states.emplace_back(new State(acceptingType));
	arg.exitState->attach(arg.exitCIndex, arg.states.back().get());
	return move(arg);
}
vector<bool> &NFA::Closure(vector<bool> &subset) const
{
	for (size_t i = 0; i < subset.size(); i++)
		if (subset[i])
			closureRecursion(i, i, subset);
	return subset;
}
vector<bool> NFA::Move(const vector<bool> &subset, size_t cIndex) const
{
	vector<bool> result(states.size(), false);
	for (size_t i = 0; i < subset.size(); i++)
	{
		if (subset[i])
		{
			for (auto tran : states[i]->transList(cIndex))
				result[tran] = true;
		}
	}
	return Closure(result);
}
void NFA::closureRecursion(size_t current, size_t checked, vector<bool> &subset) const
{
	if (current > checked)
		subset[current] = true;
	else if (!subset[current] || checked == current) // note that this fails if there is an epsilon loop
	{
		subset[current] = true;
		for (auto tran : states[current]->transList(EPSILON))
			closureRecursion(tran, checked, subset);
	}
}
NFA NFA::Concatenate(NFA &&lhs, NFA &&rhs)
{
	if (!rhs.exitState)
		return move(lhs);
	if (!lhs.exitState)
		return move(rhs);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 1);
	lhs.exitState->attach(lhs.exitCIndex, rhs.states[0].get());
	for (auto &state : lhs.states)
		result.states.push_back(move(state));
	for (auto &state : rhs.states)
		result.states.push_back(move(state));
	result.exitCIndex = rhs.exitCIndex;
	result.exitState = rhs.exitState;
	return result;
}
NFA NFA::Or(NFA &&lhs, NFA &&rhs)
{
	if (!rhs.exitState)
		return move(lhs);
	if (!lhs.exitState)
		return move(rhs);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 3);
	pState in(new State), out(new State);
	in->attach(EPSILON, lhs.states[0].get());
	in->attach(EPSILON, rhs.states[0].get());
	lhs.exitState->attach(lhs.exitCIndex, out.get());
	rhs.exitState->attach(rhs.exitCIndex, out.get());
	result.states.push_back(move(in));
	for (auto &state : lhs.states)
		result.states.push_back(move(state));
	for (auto &state : rhs.states)
		result.states.push_back(move(state));
	result.exitCIndex = EPSILON;
	result.exitState = out.get();
	result.states.push_back(move(out));
	return result;
}
NFA NFA::Star(NFA &&arg)
{
	if (!arg.exitState)
		return NFA();
	NFA result;
	result.states.reserve(arg.states.size() + 2);
	pState hub(new State);
	hub->attach(EPSILON, arg.states[0].get());
	arg.exitState->attach(arg.exitCIndex, hub.get());
	result.exitCIndex = EPSILON;
	result.exitState = hub.get();
	result.states.push_back(move(hub));
	for (auto &state : arg.states)
		result.states.push_back(move(state));
	return result;
}
NFA NFA::Plus(NFA &&arg)
{
	if (!arg.exitState)
		return NFA();
	NFA result;
	result.states.reserve(arg.states.size() + 3);
	pState in(new State), out(new State);
	in->attach(EPSILON, arg.states[0].get());
	arg.exitState->attach(arg.exitCIndex, out.get());
	out->attach(EPSILON, in.get());
	result.states.push_back(move(in));
	for (auto &state : arg.states)
		result.states.push_back(move(state));
	result.exitCIndex = EPSILON;
	result.exitState = out.get();
	result.states.push_back(move(out));
	return result;
}
NFA NFA::Merge(vector<NFA> &&nfas)
{
	NFA result;
	pState in(new State);
	for (auto &nfa : nfas)
		in->attach(EPSILON, nfa.states[0].get());
	result.states.push_back(move(in));
	for (auto &nfa : nfas)
		for (auto &state : nfa.states)
			result.states.emplace_back(move(state));
	for (size_t i = 0; i < result.states.size(); i++)
		result.states[i]->assignNum(i);
	return result;
}
size_t NFA::Size() const
{
	return states.size();
}
size_t NFA::Accepting(const vector<bool> &subset) const
{
	size_t result = states.size() + 1;
	for (size_t i = 0; i < states.size(); i++)
	{
		if (subset[i])
		{
			size_t acceptingType = states[i]->AcceptingType();
			if (acceptingType && acceptingType < result)
				result = acceptingType;
		}
	}
	if (result == states.size() + 1)
		return 0;
	return result;
}
void NFA::State::attach(size_t cIndex, NFA::State *to)				// all references need to be adjusted to give cIndex instead of c
{
	transitions.push_back({ cIndex, to });
}
void NFA::State::assignNum(size_t num)
{
	stateNum = num;
}
vector<size_t> NFA::State::transList(size_t cIndex) const		// all references need to be adjusted to give cIndex instead of c
{
	vector<size_t> result;
	result.reserve(transitions.size());
	for (auto trans : transitions)
		if (trans.cIndex == cIndex)
			result.push_back(trans.state->stateNum);
	return result;
}
size_t NFA::State::AcceptingType() const
{
	return accepting;
}
size_t NFA::charIndex(char c)
{
	vector<char>::const_iterator it = std::find(alphabet.begin(), alphabet.end(), c);
	size_t index = it - alphabet.begin();
	if (it == alphabet.end())
		alphabet.push_back(c);
	return index;
}

DFA::DFA(const NFA &nfa)
{
	vector<vector<bool>> states;
	vector<bool> stateSet(nfa.Size(), false);
	stateSet[0] = true;
	states.push_back(move(nfa.Closure(stateSet)));
	stateInfo.emplace_back();
	stateInfo[0].accepting = nfa.Accepting(states[0]);
	for (size_t stateIndex = 0; stateIndex < states.size(); stateIndex++)
	{
		for (size_t charIndex = 1; charIndex < NFA::AlphabetSize(); charIndex++)
		{
			stateSet = nfa.Move(states[stateIndex], charIndex);
			if (isNonempty(stateSet))
			{
				for (size_t prevStateIndex = 0;; prevStateIndex++)
				{
					if (prevStateIndex == states.size())
					{
						states.push_back(move(stateSet));
						stateInfo.emplace_back();
						stateInfo[prevStateIndex].accepting = nfa.Accepting(states[prevStateIndex]);
						stateInfo[stateIndex].transitions[charIndex] = prevStateIndex + 1;
						break;
					}
					else if (stateSet == states[prevStateIndex])
					{
						stateInfo[stateIndex].transitions[charIndex] = prevStateIndex + 1;
						break;
					}
				}
			}
			else
				stateInfo[stateIndex].transitions[charIndex] = 0;
		}
	}
}
DFA::DFA(const DFA &dfa)
{
	struct transition
	{
		size_t fromOld, fromNew, to;
		bool marked;
	};
	vector<size_t> states;
	states.reserve(dfa.stateInfo.size());
	size_t numStates = 1;
	bool nonAccepting = false;
	for (auto info : dfa.stateInfo)
	{
		if (!info.accepting)
			nonAccepting = true;
		if (info.accepting > numStates)
			numStates = info.accepting;
	}
	if (nonAccepting)
		numStates++;
	size_t offset = numStates - dfa.stateInfo[0].accepting;
	for (auto info : dfa.stateInfo)
		states.push_back((info.accepting + offset) % numStates + 1);
	bool consistent = false;
	while (!consistent)
	{
		consistent = true;
		for (size_t charIndex = 1; charIndex < NFA::AlphabetSize(); charIndex++)
		{
			vector<transition> transitions;
			transitions.reserve(states.size());
			vector<size_t> currentTransVals(numStates);
			for (size_t dfaState = states.size(); dfaState-- > 0;)
			{
				size_t trans = (dfa.stateInfo[dfaState].transitions[charIndex] == 0) ?
					0 : states[dfa.stateInfo[dfaState].transitions[charIndex] - 1];
				currentTransVals[states[dfaState] - 1] = trans;
				transitions.push_back({ dfaState, states[dfaState], trans });
			}
			for (size_t i = transitions.size(); i-- > 0;)
			{
				if (transitions[i].marked == false)
				{
					if (transitions[i].to != currentTransVals[transitions[i].fromNew - 1])
					{
						consistent = false;
						states[transitions[i].fromOld] = ++numStates;
						currentTransVals[transitions[i].fromNew - 1] = transitions[i].to; //experimental: num_states
					}
					for (size_t j = i; j-- > 0;)
					{
						if ((transitions[j].marked == false) && (transitions[j].fromNew == transitions[i].fromNew) &&
							(transitions[j].to == currentTransVals[transitions[j].fromNew - 1]))
						{
							transitions[j].marked = true;
							states[transitions[j].fromOld] = states[transitions[i].fromOld];
						}
					}
				}
			}
		}
	}
	stateInfo = vector<StateInfo>(numStates);
	for (size_t i = 0; i < states.size(); i++)
	{
		stateInfo[states[i] - 1].accepting = dfa.stateInfo[i].accepting;
		for (size_t j = 1; j < NFA::AlphabetSize(); j++)
			stateInfo[states[i] - 1].transitions[j] = (dfa.stateInfo[i].transitions[j] == 0) ? 0 : states[dfa.stateInfo[i].transitions[j] - 1];
	}
}
bool DFA::isNonempty(const vector<bool> &subset)				// should be static
{
	for (auto element : subset)
		if (element == true)
			return true;
	return false;
}

void DFA::PrintStates() const
{
	for (size_t i = 0; i < stateInfo.size(); i++)
	{
		std::cout << "State " << i + 1 << ": ";
		if (stateInfo[i].accepting)
			std::cout << "Accepts " << DFA::Types[stateInfo[i].accepting - 1];
		std::cout << std::endl;
		for (size_t j = 1; j < NFA::AlphabetSize(); j++)
			if (stateInfo[i].transitions[j] != 0)
				std::cout << "\tMove(" << i + 1 << ", " << NFA::Alphabet(j) << ") = " << stateInfo[i].transitions[j] << std::endl;
	}
}
void DFA::PrintHeader(std::ostream &out) const
{
	out << "#include <iostream>\n"
		"#include <istream>\n"
		"#include <memory>\n"
		"#include <string>\n"
		"#include <vector>\n\n"
		"using std::cin;\n"
		"using std::cout;\n"
		"using std::endl;\n"
		"using std::move;\n"
		"using std::istream;\n"
		"using std::ostream;\n"
		"using std::string;\n"
		"using std::vector;\n"
		"class Terminal;\n"
		"typedef string::const_iterator Iterator;\n"
		"typedef std::unique_ptr<Terminal> pTerminal;\n\n"
		"class Lexer\n"
		"{\n"
		"public:\n"
		"\tstruct Error\n"
		"\t{\n"
		"\t\tstring Token;\n"
		"\t};\n\n"
		"\tLexer(istream &in) : in(in) {}\n"
		"\tbool CreateTokens();\n"
		"\tvector<pTerminal> GetTokens() { return move(tokens); };\n"
		"\tError GetErrorReport() { return move(err); }\n"
		"private:\n"
		"\tenum Type { INVALID";
	for (const auto &type : Types)
		out << ", " << ToUpper(type);
	out << " };\n\n";
	for (size_t i = 1; i <= stateInfo.size(); i++)
		out << "\tstatic Type State_" << i << "(Iterator &it, Iterator end);\n";
	out << "\n\tistream &in;\n"
		"\tvector<pTerminal> tokens;\n"
		"\tError err;\n"
		"};\n\n"
		"class Terminal\n"
		"{\n"
		"public:\n"
		"\tvirtual ~Terminal() = 0 {}\n"
		"\tfriend ostream &operator<<(ostream &os, const Terminal &term) { return term.print(os); }\n"
		"private:\n"
		"\tvirtual ostream &print(ostream &os) const = 0;\n"
		"};\n";
	for (const auto &type : Types)
		out << "class " << type << " : public Terminal\n"
			"{\n"
			"public:\n"
			"\tUpper(string &&value) : value(move(value)) {}\n"
			"\t~Upper() = default;\n"
			"private:\n"
			"\tostream &print(ostream &os) const { return os << \"" << ToUpper(type) << "[\" << value << ']'; }\n"
			"\tconst string value;\n"
			"};\n";

	out << "\n\nbool Lexer::CreateTokens()\n"
		"{\n"
		"\tstring word;\n"
		"\twhile (in >> word)\n"
		"\t{\n"
		"\t\tIterator begin = word.begin(), it = begin, end = word.end();\n"
		"\t\tdo\n"
		"\t\t{\n"
		"\t\t\tType type = State_1(it, end);\n"
		"\t\t\tswitch (type)\n"
		"\t\t\t{\n";
	for (const auto &type : Types)
		out << "\t\t\tcase " << ToUpper(type) << ":\n"
			"\t\t\t\ttokens.emplace_back(new " << type << "(string(begin, it)));\n"
			"\t\t\t\tbreak;\n";
	out << "\t\t\tdefault:\n"
		"\t\t\t\terr = { string(begin, end) };\n"
		"\t\t\t\treturn false;\n"
		"\t\t\t}\n"
		"\t\t\tbegin = it;\n"
		"\t\t} while (it != end);\n"
		"\t}\n"
		"\treturn = true;\n"
		"}";
}
void DFA::PrintDefinitions(std::ostream &out) const
{
	for (size_t i = 0; i < stateInfo.size(); i++)
	{
		out << "\nLexer::Type Lexer::State_" << i + 1 << "(Iterator &it, Iterator end)\n"
			"{\n"
			"\tif (it != end)\n"
			"\t{\n";
		if (stateInfo[i].accepting)
		{
			out << "\t\tIterator cont = it;\n"
				"\t\tbool contValid;\n"
				"\t\tswitch (*cont++)\n"
				"\t\t{\n";
			size_t size = stateInfo[i].transitions.size();
			vector<bool> marked(size, false);
			for (size_t j = 1; j < size; j++) {
				size_t transition = stateInfo[i].transitions[j];
				if (!marked[j] && transition) {
					for (size_t k = j; k < size; k++) {
						if (stateInfo[i].transitions[k] == transition) {
							marked[k] = true;
							out << "\t\tcase '" << NFA::Alphabet(k) << "':\n";
						}
					}
					out << "\t\t\tcontValid = State_" << transition << "(cont, end);\n"
						"\t\t\tbreak;\n";
				}
			}
			out << "\t\tdefault:\n"
				"\t\t\treturn " << ToUpper(Types[stateInfo[i].accepting - 1]) << ";\n"
				"\t\t}\n"
				"\t\tif (contValid)\n"
				"\t\t\tit = cont;\n"
				"\t}\n"
				"\treturn " << ToUpper(Types[stateInfo[i].accepting - 1]) << ";\n";
		}
		else
		{
			out << "\t\tswitch (*it++)\n"
				"\t\t{\n";
			size_t size = stateInfo[i].transitions.size();
			vector<bool> marked(size, false);
			for (size_t j = 1; j < size; j++) {
				size_t transition = stateInfo[i].transitions[j];
				if (!marked[j] && transition) {
					for (size_t k = j; k < size; k++) {
						if (stateInfo[i].transitions[k] == transition) {
							marked[k] = true;
							out << "\t\tcase '" << NFA::Alphabet(k) << "':\n";
						}
					}
					out << "\t\t\treturn State_" << transition << "(it, end);\n";
				}
			}
			out << "\t\t}\n"
				"\t}\n"
				"\treturn INVALID;\n";
		}
		out << "}";
	}
}

Iterator &Iterator::operator++()
{
	if (*it == '\\')
		it++;
	it++;
	return *this;
}
Iterator Iterator::operator++(int)
{
	Iterator temp = *this;
	++*this;
	return temp;
}
char Iterator::operator*() const
{
	char c = *it;
	if (c == '(' || c == ')' || c == '*' || c == '|')
		return c;
	return '\0';
}
bool Iterator::IsChar() const
{
	return **this == '\0';
}
char Iterator::C() const
{
	if (*it == '\\')
		return *(it + 1);
	return *it;
}

Tree::Tree(const std::string &input)
{
	Iterator it = input.begin(), end = input.end();
	if (it == end)
		throw "Tree::Tree 1: Syntax Error!";
	else if (*it == '(' || it.IsChar())
	{
		node = pNode(new Q(it, end));
		if (it != end)
			throw "Tree::Tree 2: Syntax Error!";
	}
	else
		throw "Tree::Tree 3: Syntax Error!";
}
Q::Q(Iterator &it, Iterator end)
{
	if (it == end)
		throw "Q::Q 1: Syntax Error!";
	else if (*it == '(' || it.IsChar())
	{
		nodes.emplace_back(new S(it, end));
		nodes.emplace_back(new R(it, end));
	}
	else
		throw "Q::Q 2: Syntax Error!";
}
NFA Q::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
R::R(Iterator &it, Iterator end)
{
	if (it == end || *it == ')');
	else if (*it == '|')
	{
		nodes.emplace_back(new Terminal('|'));
		it++;
		nodes.emplace_back(new S(it, end));
		nodes.emplace_back(new R(it, end));
	}
	else
		throw "R::R 1: Syntax Error!";
}
NFA R::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return NFA::Or(std::move(nfa), nodes[2]->GenNfa(nodes[1]->GenNfa()));
}
S::S(Iterator &it, Iterator end)
{
	if (it == end)
		throw "S::S 1: Syntax Error!";
	else if (*it == '(' || it.IsChar())
	{
		nodes.emplace_back(new U(it, end));
		nodes.emplace_back(new T(it, end));
	}
	else
		throw "S::S 2: Syntax Error!";
}
NFA S::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
T::T(Iterator &it, Iterator end)
{
	if (it == end || *it == '|' || *it == ')');
	else if (*it == '(' || it.IsChar())
	{
		nodes.emplace_back(new U(it, end));
		nodes.emplace_back(new T(it, end));
	}
	else
		throw "T::T 1: Syntax Error!";
}
NFA T::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return nodes[1]->GenNfa(NFA::Concatenate(std::move(nfa), nodes[0]->GenNfa()));
}
U::U(Iterator &it, Iterator end)
{
	if (it == end)
		throw "U::U 1: Syntax Error!";
	else if (*it == '(' || it.IsChar())
	{
		nodes.emplace_back(new W(it, end));
		nodes.emplace_back(new V(it, end));
	}
	else
		throw "U::U 2: Syntax Error!";
}
NFA U::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
V::V(Iterator &it, Iterator end)
{
	if (it == end || *it == '|' || *it == '(' || *it == ')' || it.IsChar());
	else if (*it == '*')
	{
		nodes.emplace_back(new Terminal('*'));
		it++;
		nodes.emplace_back(new V(it, end));
	}
	else
		throw "V::V 1: Syntax Error!";
}
NFA V::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return NFA::Star(nodes[1]->GenNfa(std::move(nfa)));
}
W::W(Iterator &it, Iterator end)
{
	if (it == end)
		throw "W::W 1: Syntax Error!";
	else if (it.IsChar())
	{
		nodes.emplace_back(new Terminal(it.C()));
		it++;
	}
	else if (*it == '(')
	{
		nodes.emplace_back(new Terminal('('));
		it++;
		nodes.emplace_back(new Q(it, end));
		if (it != end && *it == ')')
			nodes.emplace_back(new Terminal(')'));
		else
			throw "W::W 2: Syntax Error!";
		it++;
	}
	else
		throw "W::W 3: Syntax Error!";
}
NFA W::GenNfa(NFA &&nfa) const
{
	if (nodes.size() == 1)
		return nodes[0]->GenNfa();
	return nodes[1]->GenNfa();
}

std::string ToUpper(const std::string &src)
{
	std::string result(src.size(), '\0');
	std::transform(src.begin(), src.end(), result.begin(), std::toupper);
	return result;
}