#include "NondeterministicFiniteAutomata.h"

#include <algorithm>
#include <utility>

std::vector<char> NFA::alphabet(1, '\0');

NFA::NFA(char c) : exitCIndex(charIndex(c))
{
    states.emplace_back(new State);
    exitState = states.back().get();
}
NFA NFA::Complete(NFA &&arg, size_t acceptingType)
{
    arg.states.emplace_back(new State(acceptingType));
    arg.exitState->attach(arg.exitCIndex, arg.states.back().get());
    return std::move(arg);
}
std::vector<bool> &NFA::Closure(std::vector<bool> &subset) const
{
    for (size_t i = 0; i < subset.size(); i++)
        if (subset[i])
            closureRecursion(i, i, subset);
    return subset;
}
std::vector<bool> NFA::Move(const std::vector<bool> &subset, size_t cIndex) const
{
    std::vector<bool> result(states.size(), false);
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
void NFA::closureRecursion(size_t current, size_t checked, std::vector<bool> &subset) const
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
        return std::move(lhs);
    if (!lhs.exitState)
        return std::move(rhs);
    NFA result;
    result.states.reserve(lhs.states.size() + rhs.states.size() + 1);
    lhs.exitState->attach(lhs.exitCIndex, rhs.states[0].get());
    for (auto &state : lhs.states)
        result.states.push_back(std::move(state));
    for (auto &state : rhs.states)
        result.states.push_back(std::move(state));
    result.exitCIndex = rhs.exitCIndex;
    result.exitState = rhs.exitState;
    return result;
}
NFA NFA::Or(NFA &&lhs, NFA &&rhs)
{
    if (!rhs.exitState)
        return std::move(lhs);
    if (!lhs.exitState)
        return std::move(rhs);
    NFA result;
    result.states.reserve(lhs.states.size() + rhs.states.size() + 3);
    pState in(new State), out(new State);
    in->attach(EPSILON, lhs.states[0].get());
    in->attach(EPSILON, rhs.states[0].get());
    lhs.exitState->attach(lhs.exitCIndex, out.get());
    rhs.exitState->attach(rhs.exitCIndex, out.get());
    result.states.push_back(std::move(in));
    for (auto &state : lhs.states)
        result.states.push_back(std::move(state));
    for (auto &state : rhs.states)
        result.states.push_back(std::move(state));
    result.exitCIndex = EPSILON;
    result.exitState = out.get();
    result.states.push_back(std::move(out));
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
    result.states.push_back(std::move(hub));
    for (auto &state : arg.states)
        result.states.push_back(std::move(state));
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
    result.states.push_back(std::move(in));
    for (auto &state : arg.states)
        result.states.push_back(std::move(state));
    result.exitCIndex = EPSILON;
    result.exitState = out.get();
    result.states.push_back(std::move(out));
    return result;
}
NFA NFA::Merge(std::vector<NFA> &&nfas)
{
    NFA result;
    pState in(new State);
    for (auto &nfa : nfas)
        in->attach(EPSILON, nfa.states[0].get());
    result.states.push_back(std::move(in));
    for (auto &nfa : nfas)
        for (auto &state : nfa.states)
            result.states.emplace_back(std::move(state));
    for (size_t i = 0; i < result.states.size(); i++)
        result.states[i]->assignNum(i);
    return result;
}
size_t NFA::Size() const
{
    return states.size();
}
size_t NFA::Accepting(const std::vector<bool> &subset) const
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
std::vector<size_t> NFA::State::transList(size_t cIndex) const		// all references need to be adjusted to give cIndex instead of c
{
    std::vector<size_t> result;
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
    std::vector<char>::const_iterator it = std::find(alphabet.begin(), alphabet.end(), c);
    size_t index = it - alphabet.begin();
    if (it == alphabet.end())
        alphabet.push_back(c);
    return index;
}
