#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include "util.h"

using namespace std;

struct Level;

string operator! (string state) {
    string orig_state  = state;
    size_t pos = state.find_last_of(':');
    if(state[pos+1] == '0') {
        state[pos+1] = '1';
    } else {
        state[pos+1] = '0';
    }

    return state;
}


bool operator == (const Action& a, const Action& b) {
    return a.name == b.name;
}

bool operator < (const Action& a, const Action& b) {
    return a.name < b.name;
}

void addToParentActions(map <string, set<Action>>& parent_actions, set<string> effects, Action& a) {
    set<string>::iterator iter;
    for (iter = effects.begin(); iter != effects.end(); ++iter) {
        //cout << "adding " << *iter << " by " << a.name << "\n";
        if(parent_actions.find(*iter) == parent_actions.end()) {
            parent_actions[*iter] = { a };
        } else {
            //set <Action> actions = parent_actions[*iter];
            //actions.insert(a);
            parent_actions[*iter].insert(a);
            //parent_actions[*iter] = actions;
        }
    }
}

string makeState(string obj, unsigned room, bool present) {
    char c[100];
    sprintf(c, "%s:%d:%d", obj.c_str(), room, present?1:0);
    return string(c); 
}

string makeAction(int from, int to) {
    char c[100];
    sprintf(c, "%s(%d,%d)", "GoThrough", from, to);
    return string(c);
}

string makeAction(string box, int from, int to) {
    char c[100];
    sprintf(c, "%s(%s,%d,%d)", "PushThrough", box.c_str(), from, to);
    return string(c); 
}

string makeAction(string state) {
    char c[100];
    sprintf(c, "%s(%s)", "Pers", state.c_str());
    return string(c); 
}

template <class T>
pair<T, T> makeMutex(const T a, const T b) {
    return pair<T, T>(a, b);
}

void addToActionMutexes(const Action& a, const Action& b, set<pair<Action, Action>>& ams) {
    //cout << "ActionMutex (" << a.name << ", " << b.name << ")\n";
    ams.insert(makeMutex(a,b));
    ams.insert(makeMutex(b,a));
}

void addToStateMutexes(string a, string b, set<pair<string, string>>& sms) {
    //cout << "StateMutex (" << a << ", " << b << ")\n";
    sms.insert(makeMutex(a,b));
    sms.insert(makeMutex(b,a));
}

void printStates(set<string>& states) {
    set<string>::iterator iter;
    for (iter = states.begin(); iter != states.end(); ++iter) {
        cout << " {" << *iter << "} , ";
    }
    cout << "\n";
}

void printActions(set<Action>& actions) {
    set<Action>::iterator iter;
    for (iter = actions.begin(); iter != actions.end(); ++iter) {
        cout << iter->name << " , ";
    }
    cout << "\n";
}

set<string> getInitFluents(string line) {
    int curr_pos = 0;

    set<string> init_states;

    while(curr_pos < line.size()) {
        size_t comma = line.find_first_of(",", curr_pos);
        int num_chars = (comma == string::npos) ? line.size()-curr_pos : comma - curr_pos;
        string state = string(line, curr_pos, num_chars);
        state += ":1";
        init_states.insert(state);
        if(comma == string::npos) break;        
        curr_pos = comma + 1;
    }
    return init_states;
}

bool checkSameParentEffect(set<Action>& pa, string state1, string state2) {
    set<Action>::iterator iter;
    for (iter = pa.begin(); iter != pa.end(); ++iter) {
      if(iter->effects.find(state1) != iter->effects.end() && 
         iter->effects.find(state2) != iter->effects.end()) 
            return true;
    }

    return false;
}

bool checkSameParentActionEffects(set<Action>& pa1, set<Action>& pa2, string state1, string state2) {
    return (checkSameParentEffect(pa1, state1, state2) || checkSameParentEffect(pa2, state1,state2));
}

set<pair<Action, Action>> makeAllPossibleActionPairs(set<Action>& pa1, set<Action>& pa2) {
    set<Action>::iterator it1, it2;
    set<pair<Action, Action>> action_pair_set;
    for(it1 = pa1.begin(); it1 != pa1.end(); ++it1) {
        for(it2 = pa2.begin(); it2 != pa2.end(); ++it2) {
            if(it1->name == it2->name) continue;
            action_pair_set.insert(makeMutex(*it1, *it2));
        }
    }

    return action_pair_set;
}

set<pair<string, string>> makeAllPossibleStatePairs(map<string, set<Action>>& parent_actions) {
    map<string, set<Action>>::iterator it1, it2;
    set<pair<string, string>> state_pair_set;
    for(it1 = parent_actions.begin(); it1 != parent_actions.end(); ++it1) {
        for(it2 = parent_actions.begin(); it2 != parent_actions.end(); ++it2) {
            if(it1->first == it2->first) continue;
            state_pair_set.insert(makeMutex(it1->first, it2->first));
        }
    }

    return state_pair_set;
}

set<string> makeGoalStates(unsigned problem_size) {
    set<string> goal_states;
    for (int i = 1 ; i < problem_size; ++i) {
        string obj = "B";
        obj += (char)(((int)'0')+i) ;
        goal_states.insert(makeState(obj, problem_size, true));
    }

    return goal_states;
}

bool areGoalFluentsPresent(unsigned problem_size, set<string> states) {
    for (int i = 1 ; i < problem_size; ++i) {
        string obj = "B";
        obj += (char)(((int)'0')+i) ;
        if(states.find(makeState(obj, problem_size, true)) == states.end()) return false;
    }
    return true;
}

bool areGoalFluentsInMutex(unsigned problem_size, set<pair<string, string>> state_mutexes) {
    set<string> goal_states = makeGoalStates(problem_size);
    set<string>::iterator iter1, iter2;
    for (iter1 = goal_states.begin(); iter1 != goal_states.end(); ++iter1) {
        for (iter2 = goal_states.begin(); iter2 != goal_states.end(); ++iter2) {
            if(*iter1 == *iter2) continue;
            if(state_mutexes.find(makeMutex(*iter1,*iter2)) != state_mutexes.end()) return true;
        }
    }

    return false;
}


set<string> GraphPlan::nextTask(unsigned& problem_size) {
    char c;
    int num_read = 0;
    string line;
    std::getline(ifile, line);
    set<string> init_states;
    if(line.size()) {
        init_states = getInitFluents(line);
        problem_size = init_states.size();
        //printStates(init_states);
    }

    return init_states;
}

double cpuTime(void)
{
    struct timeval cputime;
    gettimeofday(&cputime, NULL);
    return static_cast<double>(cputime.tv_sec) + (1.e-6)*static_cast<double>(cputime.tv_usec);
}

