#include "RegexSyntaxTree.h"

#include <utility>
#include <vector>

using namespace synTree;


namespace synTree {
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

    class Terminal : public Node
    {
    public:
        Terminal(char symbol_) : symbol(symbol_) {}
        NFA GenNfa(NFA &&nfa = NFA()) const { return NFA(symbol); }
    private:
        const char symbol;
    };
    class NonTerminal : public Node
    {
    protected:
        std::vector<pNode> nodes;
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
