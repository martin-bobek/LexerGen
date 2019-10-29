#ifndef REGEX_SYNTAX_TREE_H__
#define REGEX_SYNTAX_TREE_H__

#include "NondeterministicFiniteAutomata.h"

#include <memory>
#include <string>

class Node;
typedef std::unique_ptr<Node> pNode;


class Iterator
{
public:
    Iterator(const std::string::const_iterator &it_) : it(it_) {}
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

#endif
