#ifndef REGEX_SYNTAX_TREE_H__
#define REGEX_SYNTAX_TREE_H__

#include "ErrorLoc.h"
#include "NondeterministicFiniteAutomata.h"

#include <exception>
#include <memory>
#include <string>


namespace synTree {
    class Node {
    public:
        virtual NFA GenNfa(NFA nfa = {}) const = 0;
        virtual ~Node() = default;
    };
}

class Tree {
public:
    Tree() = default;
    explicit Tree(const std::string &input);

    NFA GenNfa(size_t acceptingType) const { return NFA::Complete(node->GenNfa(), acceptingType); }
    operator bool() const { return (bool)node; }

private:
    std::unique_ptr<synTree::Node> node;
};

class RegexParserError : public std::runtime_error {
public:
    RegexParserError(const std::string &msg, ErrorLoc loc) : std::runtime_error(createMessage(msg, loc)) {}
private:
    static std::string createMessage(const std::string &msg, ErrorLoc loc);
};

#endif
