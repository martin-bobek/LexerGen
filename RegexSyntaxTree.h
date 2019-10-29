#ifndef REGEX_SYNTAX_TREE_H__
#define REGEX_SYNTAX_TREE_H__

#include "NondeterministicFiniteAutomata.h"

#include <memory>
#include <string>


namespace synTree {
    class Node
    {
    public:
        virtual NFA GenNfa(NFA &&nfa = NFA()) const = 0;
    };

    typedef std::unique_ptr<Node> pNode;
}

class Tree
{
public:
    Tree() = default;
    explicit Tree(const std::string &input);
    NFA GenNfa(size_t acceptingType) const { return NFA::Complete(node->GenNfa(), acceptingType); }
    operator bool() const { return (bool)node; }
private:
    synTree::pNode node;
};

#endif
