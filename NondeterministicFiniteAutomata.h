#ifndef NONDETERMINISTIC_FINITE_AUTOMATA_H__
#define NONDETERMINISTIC_FINITE_AUTOMATA_H__

#include <memory>
#include <vector>

namespace nfa {
    class NfaState;
    using pNfaState = std::unique_ptr<NfaState>;
};


class NFA {
public:
    static constexpr size_t EPSILON = 0;

    NFA() = default;
    NFA(char c);

    NFA(NFA &&) = default;
    NFA(const NFA &) = delete;
    NFA &operator=(NFA &&) = default;
    NFA &operator=(const NFA &) = delete;

    size_t Accepting(const std::vector<bool> &subset) const;
    std::vector<bool> &Closure(std::vector<bool> &subset) const;
    std::vector<bool> Move(const std::vector<bool> &subset, size_t cIndex) const;
    size_t Size() const { return states.size(); }

    static NFA Complete(NFA arg, size_t acceptingType);
    static NFA Concatenate(NFA lhs, NFA rhs);
    static NFA Merge(std::vector<NFA> nfas);
    static NFA Or(NFA lhs, NFA rhs);
    static NFA Plus(NFA arg);
    static NFA Star(NFA arg);

    static char Alphabet(size_t index) { return alphabet[index]; }
    static size_t AlphabetSize() { return alphabet.size(); }

private:
    void closureRecursion(size_t current, size_t checked, std::vector<bool> &subset) const;

    static size_t charIndex(char c);

    std::vector<nfa::pNfaState> states;
    nfa::NfaState *exitState;
    size_t exitCIndex;

    static std::vector<char> alphabet;
};

namespace nfa {
    struct Transition {
        Transition(NfaState *state_, size_t cIndex_) : state(state_), cIndex(cIndex_) {}

        NfaState *state;
        size_t cIndex;
    };

    class NfaState {
    public:
        NfaState(size_t accepting_ = 0) : accepting(accepting_) {}

        void Attach(size_t cIndex, NfaState *to) { transitions.emplace_back(to, cIndex); }
        void AssignNum(size_t num) { stateNum = num; }
        std::vector<size_t> TransList(size_t cIndex) const;
        size_t AcceptingType() const { return accepting; }

    private:
        size_t accepting;
        size_t stateNum;
        std::vector<Transition> transitions;
    };
}

#endif
