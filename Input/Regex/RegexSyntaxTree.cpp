#include "RegexSyntaxTree.h"

#include <utility>
#include <vector>

using namespace synTree;


namespace synTree {
    class Iterator {
    public:
        Iterator(std::string::const_iterator it_) noexcept : it(std::move(it_)) {}

        char Char() const;
        bool IsChar() const { return **this == '\0'; }

        Iterator &operator++();
        Iterator operator++(int);
        char operator*() const;
        bool operator==(const Iterator &rhs) const { return it == rhs.it; }
        bool operator!=(const Iterator &rhs) const { return it != rhs.it; }

    private:
        std::string::const_iterator it;
    };

    class Terminal : public Node {
    public:
        Terminal(char symbol_) noexcept : symbol(symbol_) {}
        NFA GenNfa(NFA) const { return symbol; }
    private:
        char symbol;
    };
    class NonTerminal : public Node {
    protected:
        std::vector<std::unique_ptr<Node>> nodes;
    };

    class Q : public NonTerminal {
    public:
        Q(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class R : public NonTerminal {
    public:
        R(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class S : public NonTerminal {
    public:
        S(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class T : public NonTerminal {
    public:
        T(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class U : public NonTerminal {
    public:
        U(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class V : public NonTerminal {
    public:
        V(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
    class W : public NonTerminal {
    public:
        W(Iterator &it, Iterator end);
        NFA GenNfa(NFA nfa) const;
    };
}


Tree::Tree(const std::string &input) {
    Iterator it = input.begin(), end = input.end();

    if (it == end)
        throw RegexParserError("Invalid syntax", ERROR_LOC());
    else if (*it == '(' || it.IsChar()) {
        node = std::make_unique<Q>(it, end);

        if (it != end)
            throw RegexParserError("Invalid syntax", ERROR_LOC());
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}

char Iterator::Char() const {
    if (*it == '\\') {
        char c = *(it + 1);

        if (c == '$')
            return '\0';
        if (c == 'n')
            return '\n';
        if (c == 's')
            return ' ';
        if (c == 't')
            return '\t';

        return c;
    }

    return *it;
}
char Iterator::operator*() const {
    char c = *it;
    if (c == '(' || c == ')' || c == '*' || c == '|')
        return c;

    return '\0';
}
Iterator &Iterator::operator++() {
    if (*it == '\\')
        it++;

    it++;
    return *this;
}
Iterator Iterator::operator++(int) {
    Iterator temp = *this;
    ++*this;
    return temp;
}

Q::Q(Iterator &it, Iterator end) {
    if (it == end)
        throw RegexParserError("Invalid syntax", ERROR_LOC());
    else if (*it == '(' || it.IsChar()) {
        nodes.push_back(std::make_unique<S>(it, end));
        nodes.push_back(std::make_unique<R>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
R::R(Iterator &it, Iterator end) {
    if (it == end || *it == ')');
    else if (*it == '|') {
        it++;
        nodes.push_back(std::make_unique<Terminal>('|'));
        nodes.push_back(std::make_unique<S>(it, end));
        nodes.push_back(std::make_unique<R>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
S::S(Iterator &it, Iterator end) {
    if (it == end)
        throw RegexParserError("Invalid syntax", ERROR_LOC());
    else if (*it == '(' || it.IsChar()) {
        nodes.push_back(std::make_unique<U>(it, end));
        nodes.push_back(std::make_unique<T>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
T::T(Iterator &it, Iterator end) {
    if (it == end || *it == '|' || *it == ')');
    else if (*it == '(' || it.IsChar()) {
        nodes.push_back(std::make_unique<U>(it, end));
        nodes.push_back(std::make_unique<T>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
U::U(Iterator &it, Iterator end) {
    if (it == end)
        throw RegexParserError("Invalid syntax", ERROR_LOC());
    else if (*it == '(' || it.IsChar()) {
        nodes.push_back(std::make_unique<W>(it, end));
        nodes.push_back(std::make_unique<V>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
V::V(Iterator &it, Iterator end) {
    if (it == end || *it == '|' || *it == '(' || *it == ')' || it.IsChar());
    else if (*it == '*') {
        it++;
        nodes.push_back(std::make_unique<Terminal>('*'));
        nodes.push_back(std::make_unique<V>(it, end));
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}
W::W(Iterator &it, Iterator end) {
    if (it == end)
        throw RegexParserError("Invalid syntax", ERROR_LOC());
    else if (it.IsChar()) {
        nodes.push_back(std::make_unique<Terminal>(it.Char()));
        it++;
    }
    else if (*it == '(') {
        it++;
        nodes.push_back(std::make_unique<Terminal>('('));
        nodes.push_back(std::make_unique<Q>(it, end));

        if (it != end && *it == ')')
            nodes.push_back(std::make_unique<Terminal>(')'));
        else
            throw RegexParserError("Invalid syntax", ERROR_LOC());

        it++;
    }
    else
        throw RegexParserError("Invalid syntax", ERROR_LOC());
}

NFA Q::GenNfa(NFA) const {
    return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
NFA R::GenNfa(NFA nfa) const {
    if (nodes.empty())
        return nfa;
    return NFA::Or(std::move(nfa), nodes[2]->GenNfa(nodes[1]->GenNfa()));
}
NFA S::GenNfa(NFA) const {
    return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
NFA T::GenNfa(NFA nfa) const {
    if (nodes.empty())
        return nfa;
    return nodes[1]->GenNfa(NFA::Concatenate(std::move(nfa), nodes[0]->GenNfa()));
}
NFA U::GenNfa(NFA) const {
    return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
NFA V::GenNfa(NFA nfa) const {
    if (nodes.empty())
        return nfa;
    return NFA::Star(nodes[1]->GenNfa(std::move(nfa)));
}
NFA W::GenNfa(NFA) const {
    if (nodes.size() == 1)
        return nodes[0]->GenNfa();
    return nodes[1]->GenNfa();
}

std::string RegexParserError::createMessage(const std::string &msg, ErrorLoc loc) {
    return "Error: \"" + msg +
        "\" in " + loc.Function +
        " at line " + std::to_string(loc.Line) +
        " of " + loc.File;
}
