#include <queue>
#include <map>

#include "NFA.h"

using namespace std;

NFA::Node::Node()
{
    ending    = false;
    links     = unordered_map<char, vector<int>>();
    gotLink   = set<std::pair<int, char>>();
    nextNodes = list<pair<int, char>>();
}

NFA::NFA(char lambda) :
    m_graph(vector<Node>()),
    m_q0(-1),
    m_lambda(lambda)
{
}

void NFA::RemoveLambda()
{
    vector<pair<pair<int, int>, char>> remainingLinks;

    for (int i = 0; i < m_graph.size(); i++)
    {
        vector<int> lambdaClosure = GetLambdaClosure(i);

        for (int j = 0; j < lambdaClosure.size(); j++)
        {
            int closureNode = lambdaClosure[j];

            if (closureNode != i)
            {
                m_graph[i].ending = m_graph[i].ending || m_graph[closureNode].ending;

                for (pair<int, char> newLink : m_graph[closureNode].nextNodes)
                {
                    if (newLink.second != m_lambda)
                    {
                        if (m_graph[i].gotLink.find(newLink) == m_graph[i].gotLink.end())
                        {
                            m_graph[i].gotLink.insert(newLink);
                            m_graph[i].nextNodes.push_back(newLink);

                            if (m_graph[i].links.find(newLink.second) == m_graph[i].links.end())
                                m_graph[i].links[newLink.second] = vector<int>();
                            
                            m_graph[i].links[newLink.second].push_back(newLink.first);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < m_graph.size(); i++)
    {
        if (m_graph[i].links.find(m_lambda) != m_graph[i].links.end())
        {
            for (int j = 0; j < m_graph[i].links[m_lambda].size(); j++)
            {
                int nextNode = m_graph[i].links[m_lambda][j];
                pair<int, char> link = make_pair(nextNode, m_lambda);
                m_graph[i].gotLink.erase(link);
                m_graph[i].nextNodes.remove(link);
            }

            if (m_graph[i].links.find(m_lambda) != m_graph[i].links.end())
            {
                m_graph[i].links[m_lambda].clear();
                m_graph[i].links.erase(m_lambda);
            }
        }
    }
}

void NFA::ToDFA()
{
    map<set<int>, int> createdNodes;
    unordered_map<int, set<int>> setsOfNodes;

    queue<int> remainingNodes;
    vector<Node> newNodes;
    vector<pair<pair<int, int>, char>> newLinks;

    int length = m_graph.size();
    
    for (int i = 0; i < m_graph.size(); i++)
    {
        set<int> newSet;
        newSet.insert(i);

        setsOfNodes[i] = newSet;
        createdNodes[newSet] = i;

        remainingNodes.push(i);
        Node crtNode = Node();
        crtNode.ending = m_graph[i].ending;
        newNodes.push_back(crtNode);
    }
    
    while (!remainingNodes.empty())
    {
        int currentSet = remainingNodes.front();
        remainingNodes.pop();

        unordered_map<char, set<int>> links;
        for (auto node : setsOfNodes[currentSet])
        {
            for (auto& letterNodePair : m_graph[node].links)
            {
                if (links.find(letterNodePair.first) == links.end())
                    links[letterNodePair.first] = set<int>();
                for (int i = 0; i < letterNodePair.second.size(); i++)
                    links[letterNodePair.first].insert(letterNodePair.second[i]);
            }
        }
        
        for (auto& letterNodePair : links)
        {
            Node newNode = Node();
            newNode.ending = false;

            for (auto node : letterNodePair.second)
                if (m_graph[node].ending)
                    newNode.ending = true;

            if (createdNodes.find(letterNodePair.second) == createdNodes.end())
            {
                createdNodes[letterNodePair.second] = length;
                setsOfNodes[length] = letterNodePair.second;
                newNodes.push_back(newNode);
                remainingNodes.push(length);
                length++;
            }

            int target = createdNodes[letterNodePair.second];
            newLinks.push_back(make_pair(make_pair(currentSet, target), letterNodePair.first));
        }
    }

    m_graph = move(newNodes);
    for (int i = 0; i < newLinks.size(); i++)
    {
        pair<pair<int, int>, char> link = newLinks[i];
        m_graph[link.first.first].nextNodes.push_back(make_pair(link.first.second, link.second));

        if (m_graph[link.first.first].links.find(link.second) == m_graph[link.first.first].links.end())
            m_graph[link.first.first].links[link.second] = vector<int>();

        m_graph[link.first.first].links[link.second].push_back(link.first.second);
        m_graph[link.first.first].gotLink.insert(make_pair(link.first.second, link.second));
    }
}

void NFA::Minimize()
{
    vector<vector<int>> reverseLinks = GetReverseLinks();
 
    bool* initialVisited = new bool[m_graph.size()];
    bool* endVisited = new bool[m_graph.size()];
    bool* visited = new bool[m_graph.size()];

    memset(initialVisited, 0, sizeof(bool) * m_graph.size());
    memset(endVisited, 0, sizeof(bool) * m_graph.size());
    memset(visited, 0, sizeof(bool) * m_graph.size());

    visited[m_q0] = true;
    initialVisited[m_q0] = true;
    queue<int> remainingNodes;
    remainingNodes.push(m_q0);
    
    while (!remainingNodes.empty())
    {
        int crtNode = remainingNodes.front();
        remainingNodes.pop();

        for (auto nextPair : m_graph[crtNode].nextNodes)
        {
            int nextNode = nextPair.first;
            if (!visited[nextNode])
            {
                visited[nextNode] = true;
                initialVisited[nextNode] = true;
                remainingNodes.push(nextNode);
            }
        }
    }

    for (int i = 0; i < m_graph.size(); i++)
    {
        if (m_graph[i].ending)
        {
            memset(visited, 0, sizeof(bool) * m_graph.size());
            
            remainingNodes.push(i);
            
            visited[i] = true;
            endVisited[i] = true;
            
            while (!remainingNodes.empty())
            {
                int crtNode = remainingNodes.front();
                remainingNodes.pop();

                for (int j = 0; j < reverseLinks[crtNode].size(); j++)
                {
                    int prevNode = reverseLinks[crtNode][j];

                    if (!visited[prevNode])
                    {
                        visited[prevNode] = true;
                        endVisited[prevNode] = true;
                        remainingNodes.push(prevNode);
                    }
                }
            }
        }
    }

    bool* keep = new bool[m_graph.size()];
    memset(keep, 0, sizeof(bool) * m_graph.size());

    for (int i = 0; i < m_graph.size(); i++)
    {
        if (initialVisited[i] && endVisited[i])
            keep[i] = true;
    }

    FilterNodes(keep);
    
    vector<set<int>> nodeGroups;
    vector<int>      whichSets;

    GetAutomataGroups(nodeGroups, whichSets);
    
    vector<Node> newGraph;
    newGraph.resize(nodeGroups.size());

    int newInitial = m_q0;

    for (int i = 0; i < nodeGroups.size(); i++)
    {
        auto& group = nodeGroups[i];
        if (!group.empty())
        {
            int node = *group.begin();
            if (node == m_q0)
                newInitial = i;
            
            newGraph[i].ending = m_graph[node].ending;

            for (auto& nodePair : m_graph[node].nextNodes)
                newGraph[i].nextNodes.push_back(make_pair(whichSets[nodePair.first], nodePair.second));
        }
    }

    m_q0 = newInitial;

    // now fill the data required for the new graph
    for (int i = 0; i < newGraph.size(); i++)
    {
        for (auto nodePair : newGraph[i].nextNodes)
        {
            newGraph[i].gotLink.insert(nodePair);

            if (newGraph[i].links.find(nodePair.second) == newGraph[i].links.end())
                newGraph[i].links[nodePair.second] = vector<int>();

            newGraph[i].links[nodePair.second].push_back(nodePair.first);
        }
    }

    m_graph = move(newGraph);
}

vector<int> NFA::GetLambdaClosure(int node)
{
    vector<int> closure = vector<int>();
    closure.push_back(node);
    bool* visitedNode = new bool[m_graph.size()];
    memset(visitedNode, 0, sizeof(bool) * m_graph.size());
    queue<int> toVisit;

    visitedNode[node] = true;
    toVisit.push(node);
    
    while (!toVisit.empty())
    {
        int current = toVisit.front();
        toVisit.pop();

        if (m_graph[current].links.find(m_lambda) != m_graph[current].links.end())
        {
            for (int i = 0; i < m_graph[current].links[m_lambda].size(); i++)
            {
                int nextNode = m_graph[current].links[m_lambda][i];
                if (!visitedNode[nextNode])
                {
                    visitedNode[nextNode] = true;
                    toVisit.push(nextNode);
                    closure.push_back(nextNode);
                }
            }
        }
    }
    delete[] visitedNode;
    visitedNode = NULL;

    return closure;
}

vector<vector<int>> NFA::GetReverseLinks()
{
    vector<vector<int>> result;
    result.resize(m_graph.size());

    for (int i = 0; i < m_graph.size(); i++)
        for (auto nodePair : m_graph[i].nextNodes)
            result[nodePair.first].push_back(i);

    return result;
}

void NFA::FilterNodes(bool* keep)
{
    unordered_map<int, int> swaps;
    int index = 0;

    for (int i = 0; i < m_graph.size(); i++)
        if (keep[i])
        {
            swaps[i] = index;
            index++;
        }

    vector<Node> newGraph;
    newGraph.resize(index);

    index = 0;
    for (int i = 0; i < m_graph.size(); i++)
        if (keep[i])
        {
            newGraph[index].ending = m_graph[i].ending;

            if (i == m_q0)
                m_q0 = index;

            for (auto nodePair : m_graph[i].nextNodes)
                if (swaps.find(nodePair.first) != swaps.end())
                {
                    pair<int, char> nextLink = make_pair(swaps[nodePair.first], nodePair.second);
                    newGraph[index].nextNodes.push_back(nextLink);
                }

            index++;
        }

    // now fill the data required for the new graph
    for (int i = 0; i < newGraph.size(); i++)
    {
        for (auto nodePair : newGraph[i].nextNodes)
        {
            newGraph[i].gotLink.insert(nodePair);
            
            if (newGraph[i].links.find(nodePair.second) == newGraph[i].links.end())
                newGraph[i].links[nodePair.second] = vector<int>();

            newGraph[i].links[nodePair.second].push_back(nodePair.first);
        }
    }

    m_graph = move(newGraph);
}

void NFA::GetAutomataGroups(vector<set<int>>& groups, 
                            vector<int>&      nodeKeys)
{
    vector<set<int>> prevP;
    vector<int> whichSet;
    prevP.resize(2);
    whichSet.resize(m_graph.size());

    for (int i = 0; i < m_graph.size(); i++)
    {
        if (m_graph[i].ending)
        {
            prevP[1].insert(i);
            whichSet[i] = 1;
        }
        else
        {
            prevP[0].insert(i);
            whichSet[i] = 0;
        }
    }
    vector<set<int>> p;
    bool same = false;

    while (!same)
    {
        for (int i = 0; i < prevP.size(); i++)
        {
            auto& subset = prevP[i];
            map<set<pair<char, int>>, vector<int>> generatedSubsets;
            for (auto& node : subset)
            {
                set<pair<char, int>> nodeSubset;
                for (auto& nodePair : m_graph[node].nextNodes)
                {
                    nodeSubset.insert(make_pair(nodePair.second, whichSet[nodePair.first]));
                }
                if (generatedSubsets.find(nodeSubset) == generatedSubsets.end())
                    generatedSubsets[nodeSubset] = vector<int>();
                generatedSubsets[nodeSubset].push_back(node);
            }

            for (auto& genSub : generatedSubsets)
            {
                set<int> crtSet = set<int>();
                for (auto& node : genSub.second)
                    crtSet.insert(node);
                p.push_back(crtSet);
            }
        }

        for (int i = 0; i < p.size(); i++)
            for (auto& node : p[i])
                whichSet[node] = i;

        if (p.size() == prevP.size())
        {
            bool sm = true;
            for (int i = 0; i < p.size(); i++)
                if (p[i] != prevP[i])
                    sm = false;

            same = sm;
        }

        prevP = move(p);
    }

    groups   = move(prevP);
    nodeKeys = move(whichSet);
}

istream& operator>>(istream& in, NFA& nfa)
{
    int nodesCount, linksCount;
    int finalStatesCount;

    in >> nodesCount >> linksCount;

    nfa.m_graph.clear();
    nfa.m_graph.resize(nodesCount);

    for (int i = 0; i < linksCount; i++)
    {
        int q1, q2;
        char c;
        in >> q1 >> q2 >> c;

        if (nfa.m_graph[q1].links.find(c) == nfa.m_graph[q1].links.end())
            nfa.m_graph[q1].links[c] = vector<int>();

        nfa.m_graph[q1].links[c].push_back(q2);
        nfa.m_graph[q1].nextNodes.push_back(make_pair(q2, c));
        nfa.m_graph[q1].gotLink.insert(make_pair(q2, c));
    }

    in >> nfa.m_q0 >> finalStatesCount;

    for (int i = 0; i < finalStatesCount; i++)
    {
        int qf;
        in >> qf;
        nfa.m_graph[qf].ending = true;
    }

    return in;
}

ostream& operator<<(ostream& out, NFA& nfa)
{
    for (int i = 0; i < nfa.m_graph.size(); i++)
    {
        out << "Node " << i;
        
        if (i == nfa.m_q0)
            out << " (initial)";
        
        if (nfa.m_graph[i].ending)
            out << " (final)";
        
        out << ": ";

        for (pair<int, char> node : nfa.m_graph[i].nextNodes)
            out << "(" 
                << (node.second == nfa.m_lambda ? "LAMBDA" : string(1, node.second))
                << " " 
                << node.first
                << ") ";

        out << endl;
    }

    return out;
}
