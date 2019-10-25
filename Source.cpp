#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#include <cctype>
#include <tuple>

using namespace std::literals::string_literals;
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
    DFA(const DFA &) = delete;
    DFA(DFA &&) = default;
    DFA &operator=(const DFA &) = delete;
    DFA &operator=(DFA &&) = default;
    static DFA Optimize(const DFA &dfa);

    size_t Size() const { return stateInfo.size(); }
    std::tuple<size_t, std::vector<size_t>> operator[](size_t state) const { return { stateInfo[state].accepting, move(stateInfo[state].transitions) }; }
private:
    DFA() = default;
    static bool isNonempty(const std::vector<bool> &subset);

    struct StateInfo
    {
        size_t accepting = 0;
        vector<size_t> transitions = vector<size_t>(NFA::AlphabetSize(), EPSILON);
    };
    vector<StateInfo> stateInfo;
};

class CodeGen
{
public:
    CodeGen(const DFA &dfa);

    CodeGen(const CodeGen &) = delete;
    CodeGen(CodeGen &&) = delete;
    CodeGen &operator=(const CodeGen &) = delete;
    CodeGen &operator=(CodeGen &&) = delete;

    static void AddType(std::string &&name);
    void PrintStates(std::ostream &out) const;
    void PrintClass(std::ostream &out) const;
    void PrintTerminals(std::ostream &out) const;
    void PrintDefinitions(std::ostream &out) const;
    void PrintSymHeader(std::ostream &out) const;
private:
    class State;
    struct Transition
    {
        Transition(const State *to, size_t charIndex) : to(to), charIndex(charIndex) {}
        const State *to;
        size_t charIndex;
    };
    typedef std::unique_ptr<State> pState;

    std::vector<pState> states;
    size_t numStates;
    static vector<std::string> types;									/// figure out a better way of doing this
};
vector<std::string> CodeGen::types = vector<std::string>();
class CodeGen::State
{
public:
    State(size_t state, size_t accepting) : oldState(state), accepting(accepting) {}
    void AddTransitions(std::vector<Transition> &&trans);
    void InitStateNum(size_t num) { newState = num; }

    bool Empty() const { return !transitions.size(); }
    void PrintTransitions(std::ostream &os) const;
    void PrintDefinition(std::ostream &out) const;
    std::string Call(bool useCont) const;
private:
    struct TransGroup
    {
        TransGroup(const State *to, std::vector<size_t> &&charIndices) : to(to), charIndices(move(charIndices)) {}
        const State *to;
        std::vector<size_t> charIndices;
    };
    size_t oldState;
    size_t newState;
    size_t accepting;
    std::vector<TransGroup> transitions;
};

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
    Tree() = default;
    explicit Tree(const std::string &input);
    NFA GenNfa(size_t acceptingType) const { return NFA::Complete(node->GenNfa(), acceptingType); }
    operator bool() const { return (bool)node; }
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

class Parser {
public:
    Parser(std::istream &in) : in(&in) {}
    bool ParseInput();
    std::vector<NFA> GetNFAs() { return std::move(nfas); }
    std::string GetError() { return std::move(error); }

    Parser(Parser &&) = default;
    Parser(const Parser &) = delete;
    Parser &operator=(Parser &&) = default;
    Parser &operator=(const Parser &) = delete;
private:
    std::istream *in;
    std::vector<NFA> nfas;
    std::string error;
};

void ErrorExit(const std::string &message);
Tree ReadTerminal(std::istream &in);
std::string ParseLine(const std::string &str);

int main(int argc, char *argv[])
{
    if (argc != 6)
        ErrorExit("Incorrect number of parameters!");

    std::ifstream in;
    if (!(in = std::ifstream(argv[1])))
        ErrorExit("Failed to open file: "s + argv[1]);

    vector<NFA> nfas;
    try {
        Tree tree;
        for (size_t i = 1; tree = ReadTerminal(in); i++)
            nfas.push_back(tree.GenNfa(i));
    }
    catch (const char *err) {
        ErrorExit(err);
    }
    in.close();

    CodeGen codeGen(DFA::Optimize(NFA::Merge(move(nfas))));
    codeGen.PrintStates(std::cout);

    std::ofstream out;
    if (!(out = std::ofstream(argv[2])))
        ErrorExit("Failed to open file: "s + argv[2]);
    codeGen.PrintSymHeader(out);

    if (!(out = std::ofstream(argv[3])))
        ErrorExit("Failed to open file: "s + argv[3]);
    codeGen.PrintTerminals(out);

    if (!(out = std::ofstream(argv[4])))
        ErrorExit("Failed to open file: "s + argv[4]);
    codeGen.PrintClass(out);

    if (!(out = std::ofstream(argv[5])))
        ErrorExit("Failed to open file: "s + argv[5]);
    codeGen.PrintDefinitions(out);
}

std::string ParseLine(const std::string &str) {
    std::stringstream stream(str);

    if (stream.get() != ':')
        ErrorExit("Lines must begin with :");

    std::string word;
    if (!(stream >> word))
        ErrorExit("Expected Terminal name after : in " + str);
    CodeGen::AddType(move(word));

    if (!(stream >> word) || word != ">")
        ErrorExit("Expected > after Terminal in " + str);

    std::string regEx;
    if (!(stream >> regEx))
        ErrorExit("Expected regular expression after Terminal name in " + str);

    if (stream >> word)
        ErrorExit("Unexpected text after regular expression in " + str);

    return regEx;
}
Tree ReadTerminal(std::istream &in) {
    std::string line;

    if (!std::getline(in, line))
        return {};

    line = ParseLine(line);
    return Tree(line);
}
void ErrorExit(const std::string &message) {
    std::cerr << message << std::endl;
    exit(1);
}

bool Parser::ParseInput() {
    try {
        Tree tree;
        for (size_t i = 1; tree = ReadTerminal(*in); i++)
            nfas.push_back(tree.GenNfa(i));
    }
    catch (const char *err) {
        error = err;
        return false;
    }

    if (nfas.empty()) {
        error = "Input file is empty!";
        return false;
    }

    return true;
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
DFA DFA::Optimize(const DFA &dfa)
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

    DFA opt;
    opt.stateInfo = vector<StateInfo>(numStates);
    for (size_t i = 0; i < states.size(); i++)
    {
        opt.stateInfo[states[i] - 1].accepting = dfa.stateInfo[i].accepting;
        for (size_t j = 1; j < NFA::AlphabetSize(); j++)
            opt.stateInfo[states[i] - 1].transitions[j] = (dfa.stateInfo[i].transitions[j] == 0) ? 0 : states[dfa.stateInfo[i].transitions[j] - 1];
    }
    return opt;
}
bool DFA::isNonempty(const vector<bool> &subset)				// should be static
{
    for (auto element : subset)
        if (element == true)
            return true;
    return false;
}

CodeGen::CodeGen(const DFA &dfa)
{
    std::vector<std::vector<size_t>> transitions;
    transitions.reserve(dfa.Size());
    for (size_t state = 0; state < dfa.Size(); state++)
    {
        auto [accepting, trans] = dfa[state];
        transitions.push_back(trans);
        states.emplace_back(new State(state + 1, accepting));
    }
    for (size_t state = 0; state < transitions.size(); state++)
    {
        std::vector<Transition> transList;
        transList.reserve(transitions.size());
        for (size_t charIndex = 1; charIndex < transitions[0].size(); charIndex++)
            if (transitions[state][charIndex])
                transList.emplace_back(states[transitions[state][charIndex] - 1].get(), charIndex);
        states[state]->AddTransitions(move(transList));
    }
    states[0]->InitStateNum(1);
    numStates = 1;
    for (size_t state = 1; state < states.size(); state++)
        if (!states[state]->Empty())
            states[state]->InitStateNum(++numStates);
}
void CodeGen::AddType(std::string &&name)
{
    types.push_back(move(name));
}
void CodeGen::PrintStates(std::ostream &os) const
{
    for (size_t i = 0; i < states.size(); i++)
        states[i]->PrintTransitions(os);
}
void CodeGen::PrintClass(std::ostream &out) const
{
    out <<
        "#ifndef LEXER_H__\n"
        "#define LEXER_H__\n\n"

        "#include <memory>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include \"Terminals.h\"\n\n"

        "class Lexer {\n"
        "public:\n"
        "    struct Error {\n"
        "        std::string Token;\n"
        "    };\n\n"

        "    Lexer(const std::string &in) : in(&in) {}\n"
        "    bool CreateTokens();\n"
        "    std::vector<pTerminal> GetTokens() { return std::move(tokens); };\n"
        "    Error GetErrorReport() { return std::move(err); }\n\n"

        "    Lexer(Lexer &&) = default;\n"
        "    Lexer &operator=(Lexer &&) = default;\n"
        "private:\n"
        "    using Iterator = std::string::const_iterator;\n"
        "    enum Type { INVALID";

    for (const auto &type : types)
        out << ", " << ToUpper(type);
    out << " };\n\n";

    for (size_t i = 1; i <= numStates; i++)
        out << "    static Type State_" << i << "(Iterator &it, Iterator end);\n";

    out <<
      "\n    const std::string *in;\n"
        "    std::vector<pTerminal> tokens;\n"
        "    Error err;\n"
        "};\n\n"

        "#endif\n";
}
void CodeGen::PrintTerminals(std::ostream &out) const
{
    out <<
        "#ifndef TERMINALS_H__\n"
        "#define TERMINALS_H__\n\n"

        "#include <memory>\n"
        "#include <ostream>\n"
        "#include <string>\n"
        "#include \"Symbol.h\"\n\n"

        "class Terminal : public Symbol {\n"
        "public:\n"
        "    virtual ~Terminal() = 0;\n"
        "    virtual bool Process(Stack &stack, SymStack &symStack, SyntaxError &err) const = 0;\n"
        "    friend std::ostream &operator<<(std::ostream &os, const Terminal &term) { return term.print(os); }\n"
        "private:\n"
        "    virtual std::ostream &print(std::ostream &os) const = 0;\n"
        "};\n"
        "inline Terminal::~Terminal() = default;\n\n";

    for (const auto &type : types) {
        out <<
            "class " << type << " : public Terminal {\n"
            "public:\n"
            "    " << type << "(std::string value) : value(std::move(value)) {}\n"
            "    bool Process(Stack &stack, SymStack &symStack, SyntaxError &err) const;\n"
            "private:\n"
            "    std::ostream &print(std::ostream &os) const { return os << \"\\033[31m" << ToUpper(type) << "[\\033[0m\" << value << \"\\033[31m]\\033[0m\"; }\n\n"
            "    const std::string value;\n"
            "};\n";
    }

    out << "\nusing pTerminal = std::unique_ptr<Terminal>;\n";
    for (const auto &type : types)
        out << "using p" << type << " = std::unique_ptr<" << type << ">;\n";

    out << "\n#endif\n";
}
void CodeGen::PrintDefinitions(std::ostream &out) const
{
    out << "#include \"Lexer.h\"\n\n"
        "bool Lexer::CreateTokens() {\n"
        "    Iterator begin = in->begin(), it = begin, end = in->end();\n\n"
        "    while (it != end) {\n"
        "        Type type = State_1(it, end);\n\n"
        "        switch (type) {\n";
    for (const auto &type : types)
        out << "        case " << ToUpper(type) << ":\n"
        "            tokens.emplace_back(new " << type << "(std::string(begin, it)));\n"
        "            break;\n";
    out << "        default:\n"
        "            err = { std::string(begin, end) };\n"
        "            return false;\n"
        "        }\n\n"
        "        begin = it;\n"
        "    }\n\n"
        "    return true;\n"
        "}\n";
    states[0]->PrintDefinition(out);
    for (size_t i = 1; i < states.size(); i++)
        if (!states[i]->Empty())
            states[i]->PrintDefinition(out);
}
void CodeGen::PrintSymHeader(std::ostream &out) const {
    out <<
        "#ifndef SYMBOL_H__\n"
        "#define SYMBOL_H__\n\n"

        "#include <memory>\n"
        "#include <stack>\n"
        "#include <string>\n"
        "#include <vector>\n\n"

        "struct SyntaxError {\n"
        "    std::string Location;\n"
        "    std::string Message;\n"
        "};\n\n"

        "class Symbol {\n"
        "public:\n"
        "    virtual ~Symbol() = 0;\n"
        "};\n"
        "inline Symbol::~Symbol() = default;\n\n"

        "using pSymbol = std::unique_ptr<Symbol>;\n"
        "using Stack = std::stack<size_t, std::vector<size_t>>;\n"
        "using SymStack = std::stack<pSymbol, std::vector<pSymbol>>;\n\n"

        "#endif\n";
}
void CodeGen::State::AddTransitions(std::vector<Transition> &&transList)
{
    std::vector<bool> marked(transList.size(), false);
    for (size_t i = 0; i < transList.size(); i++)
    {
        if (!marked[i])
        {
            transitions.emplace_back(transList[i].to, std::vector<size_t>(1, transList[i].charIndex));
            for (size_t j = i + 1; j < transList.size(); j++)
            {
                if (transList[j].to == transList[i].to)
                {
                    marked[j] = true;
                    transitions.back().charIndices.push_back(transList[j].charIndex);
                }
            }
        }
    }
}
std::string CodeGen::State::Call(bool useCont) const
{
    if (transitions.size() != 0)
    {
        std::string result = "State_";
        result += std::to_string(newState);
        if (useCont)
            result += "(cont, end)";
        else
            result += "(it, end)";
        return result;
    }
    return ToUpper(types[accepting - 1]);
}
void CodeGen::State::PrintTransitions(std::ostream &os) const
{
    os << "State " << oldState << ':';
    if (accepting)
        os << " Accepts " << types[accepting - 1];
    os << '\n';
    for (const TransGroup &transGroup : transitions)
    {
        os << "    { ";
        size_t i = 0;
        while (true)
        {
            char c = NFA::Alphabet(transGroup.charIndices[i]);
            if (c == '\n')
                os << "\\n";
            else if (c == ' ')
                os << "\\s";
            else if (c == '\t')
                os << "\\t";
            else
                os << c;
            if (++i == transGroup.charIndices.size())
                break;
            os << ", ";
        }
        os << " } -> " << transGroup.to->oldState << '\n';
    }
}
void CodeGen::State::PrintDefinition(std::ostream &out) const
{
    out << "Lexer::Type Lexer::State_" << newState << "(Iterator &it, Iterator end) {\n"
        "    if (it != end) {\n";
    if (accepting)
        out << "        Iterator cont = it;\n"
        "        Type contValid = INVALID;\n\n"
        "        switch (*cont++) {\n";
    else
        out << "        switch (*it++) {\n";
    for (const TransGroup &transition : transitions)
    {
        for (size_t charIndex : transition.charIndices) {
            out << "        case '";

            char c = NFA::Alphabet(charIndex);
            if (c == '\n')
                out << "\\n";
            else if (c == '\t')
                out << "\\t";
            else
                out << c;

            out << "':\n";
        }
        if (accepting)
            out << "            contValid = " << transition.to->Call(true) << ";\n"
            "            break;\n";
        else
            out << "            return " << transition.to->Call(false) << ";\n";
    }
    if (accepting)
        out << "        }\n\n"
        "        if (contValid != INVALID) {\n"
        "            it = cont;\n"
        "            return contValid;\n"
        "        }\n"
        "    }\n\n"
        "    return " << ToUpper(types[accepting - 1]) << ";\n";
    else
        out << "        }\n"
        "    }\n\n"
        "    return INVALID;\n";
    out << "}\n";
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
    {
        char c = *(it + 1);
        if (c == '$')
            return '\0';
        if (c == 'n')
            return '\n';
        if (c == 's')
            return ' ';
        if (c == 't')
            return '\t';
        return *(it + 1);
    }
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
    std::transform(src.begin(), src.end(), result.begin(), (int(*)(int))std::toupper);
    return result;
}
