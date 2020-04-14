#pragma once

#include <utility>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <istream>
#include <ostream>

class NFA
{
private:

    struct Node
    {
    public:

        Node();

    public:

        bool                                       ending;
        std::unordered_map<char, std::vector<int>> links;
        std::set<std::pair<int, char>>             gotLink;
        std::list<std::pair<int, char>>            nextNodes;
    };

public:

    NFA(char);

    void RemoveLambda();
    void ToDFA();
    void Minimize();

    friend std::istream& operator>>(std::istream&, NFA&);
    friend std::ostream& operator<<(std::ostream&, NFA&);

private:

    std::vector<int>              GetLambdaClosure(int);
    std::vector<std::vector<int>> GetReverseLinks();
    void                          FilterNodes(bool*);
    void                          GetAutomataGroups(std::vector<std::set<int>>&,
                                                    std::vector<int>&);

private:

    std::vector<Node> m_graph;
    unsigned int      m_q0;
    char              m_lambda;
};