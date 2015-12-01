//
// Created by vladimir on 08.08.2015.
//

#ifndef SOCKET_DFA_H
#define SOCKET_DFA_H

#include <map>
#include <utility>

template <class S, class M> class DFA {
    std::map<std::pair<S, M>, S> _table;
    std::size_t _it = 0;
    S _currentState;

    S _begin, _end;

    public:
    DFA(const S &beginState, const S &endState) : _begin(beginState), _end(endState) { _currentState = beginState; }

    void Add(const std::pair<S, M> &pair, const S &state) {
        _table.insert(std::make_pair(pair, state)); // = state;
    }
    void Transition(const M &token) {
        auto nextState = _table.find(std::make_pair(_currentState, token));
        if (nextState == _table.end())
            throw std::runtime_error("transition does not exist");
        _currentState = nextState->second;
    }

    const S &State() const { return _currentState; }
    std::size_t StateNumber() const { return _table.size(); }
};

#endif // SOCKET_DFA_H
