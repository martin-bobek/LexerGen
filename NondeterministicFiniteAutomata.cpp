#include "NondeterministicFiniteAutomata.h"

#include <algorithm>
#include <utility>

using namespace nfa;


std::vector<char> NFA::alphabet(1, '\0');

NFA::NFA(char exitChar) : exitCIndex(charIndex(exitChar)) {
    auto &state = states.emplace_back(std::make_unique<NfaState>());
    exitState = state.get();
}

size_t NFA::Accepting(const std::vector<bool> &subset) const {
    size_t result = states.size() + 1;

    for (size_t i = 0; i < states.size(); i++) {
        if (subset[i]) {
            size_t acceptingType = states[i]->AcceptingType();

            if (acceptingType && acceptingType < result)
                result = acceptingType;
        }
    }

    if (result == states.size() + 1)
        return 0;

    return result;
}
std::vector<bool> &NFA::Closure(std::vector<bool> &subset) const {
    for (size_t i = 0; i < subset.size(); i++)
        if (subset[i])
            closureRecursion(i, i, subset);

    return subset;
}
std::vector<bool> NFA::Move(const std::vector<bool> &subset, size_t cIndex) const {
    std::vector<bool> result(states.size(), false);

    for (size_t i = 0; i < subset.size(); i++)
        if (subset[i])
            for (auto tran : states[i]->TransList(cIndex))
                result[tran] = true;

    return Closure(result);
}

NFA NFA::Complete(NFA arg, size_t acceptingType) {
    auto &state = arg.states.emplace_back(std::make_unique<NfaState>(acceptingType));
    arg.exitState->Attach(arg.exitCIndex, state.get());
    return arg;
}
NFA NFA::Concatenate(NFA lhs, NFA rhs) {
    if (!rhs)
        return lhs;
    if (!lhs)
        return rhs;

    lhs.exitState->Attach(lhs.exitCIndex, rhs.states[0].get());
    lhs.exitState = rhs.exitState;
    lhs.exitCIndex = rhs.exitCIndex;

    lhs.states.reserve(lhs.states.size() + rhs.states.size() + 1);
    std::move(rhs.states.begin(), rhs.states.end(), std::back_inserter(lhs.states));

    return lhs;
}
NFA NFA::Merge(std::vector<NFA> nfas) {
    auto in = std::make_unique<NfaState>();
    size_t size = 1;

    for (auto &nfa : nfas) {
        in->Attach(EPSILON, nfa.states[0].get());
        size += nfa.states.size();
    }

    NFA result;
    result.states.reserve(size);
    result.states.push_back(std::move(in));

    for (auto &nfa : nfas)
        std::move(nfa.states.begin(), nfa.states.end(), std::back_inserter(result.states));

    for (size_t i = 0; i < result.states.size(); i++)
        result.states[i]->AssignNum(i);

    return result;
}
NFA NFA::Or(NFA lhs, NFA rhs) {
    if (!rhs)
        return lhs;
    if (!lhs)
        return rhs;

    auto in = std::make_unique<NfaState>();
    in->Attach(EPSILON, lhs.states[0].get());
    in->Attach(EPSILON, rhs.states[0].get());

    auto out = std::make_unique<NfaState>();
    lhs.exitState->Attach(lhs.exitCIndex, out.get());
    rhs.exitState->Attach(rhs.exitCIndex, out.get());

    NFA result;
    result.states.reserve(lhs.states.size() + rhs.states.size() + 3);
    result.states.push_back(std::move(in));

    std::move(lhs.states.begin(), lhs.states.end(), std::back_inserter(result.states));
    std::move(rhs.states.begin(), rhs.states.end(), std::back_inserter(result.states));

    result.exitState = out.get();
    result.states.push_back(std::move(out));
    result.exitCIndex = EPSILON;

    return result;
}
NFA NFA::Plus(NFA arg) {
    if (!arg)
        return {};

    auto in = std::make_unique<NfaState>();
    in->Attach(EPSILON, arg.states[0].get());

    auto out = std::make_unique<NfaState>();
    out->Attach(EPSILON, in.get());
    arg.exitState->Attach(arg.exitCIndex, out.get());

    NFA result;
    result.states.reserve(arg.states.size() + 3);
    result.states.push_back(std::move(in));

    std::move(arg.states.begin(), arg.states.end(), std::back_inserter(result.states));

    result.exitState = out.get();
    result.states.push_back(std::move(out));
    result.exitCIndex = EPSILON;

    return result;
}
NFA NFA::Star(NFA arg) {
    if (!arg)
        return {};

    auto hub = std::make_unique<NfaState>();
    hub->Attach(EPSILON, arg.states[0].get());
    arg.exitState->Attach(arg.exitCIndex, hub.get());

    NFA result;
    result.states.reserve(arg.states.size() + 2);

    result.exitState = hub.get();
    result.states.push_back(std::move(hub));
    result.exitCIndex = EPSILON;

    std::move(arg.states.begin(), arg.states.end(), std::back_inserter(result.states));

    return result;
}

void NFA::closureRecursion(size_t current, size_t checked, std::vector<bool> &subset) const {
    if (current > checked)
        subset[current] = true;
    else if (!subset[current] || checked == current) { // note that this fails if there is an epsilon loop
        subset[current] = true;

        for (auto tran : states[current]->TransList(EPSILON))
            closureRecursion(tran, checked, subset);
    }
}

size_t NFA::charIndex(char c) {
    auto it = std::find(alphabet.begin(), alphabet.end(), c);
    size_t index = it - alphabet.begin();

    if (it == alphabet.end())
        alphabet.push_back(c);

    return index;
}


std::vector<size_t> NfaState::TransList(size_t cIndex) const {
    std::vector<size_t> result;
    result.reserve(transitions.size());

    for (auto trans : transitions)
        if (trans.cIndex == cIndex)
            result.push_back(trans.state->stateNum);

    return result;
}
