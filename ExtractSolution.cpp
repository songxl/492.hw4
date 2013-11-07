#include <limits.h>
#include <iostream>
#include <stdio.h>
#include <queue>
#include <map>
#include <set>
#include "util.h"

using namespace std;

/*
ExtractSolution algo : 
This API provides the user with extractSolution() function.
This requires the Level, goal_states and solution_map as parameters.

This can be viewed as a CSP problem.
CSP {
    Variables : states at each level
    Domains : the possible actions that have effects as these states
}

We use Backtracking with AC3 to come up with the assignment to Variables similar to what we did for HW3 (Sodoku puzzle solver)

Please Note : This CSP formulation is different from the one mentioned in the book where Variables are Actions at each level and domians are {include, exclude}
Also, note that we dont make use of no_goods to keep it simple.
In order to stop the expansion of planning graph, you may rely upon the condition that the two levels have same literals and same mutexes.
*/


bool ExtractSolution::AC3(map<string, set<Action>>& domains_map, Level& l, map<int, set<Action>> sol) {

    queue<string> q;
    set<string> q_set;
    map<string, set<Action>>::iterator it;

    for (it = domains_map.begin(); it != domains_map.end(); ++it) {
        q.push(it->first);
        q_set.insert(it->first);
    }

    while(q.size()) {
        if(domains_map[q.front()].size() == 1) {
            if(!Revise(domains_map, l.action_mutexes, *(domains_map[q.front()].begin()), q, q_set)) return false;
        }
        q.pop();
    }

    for (it = domains_map.begin(); it != domains_map.end(); ++it) {
        if(it->second.size() == 1) {
            sol[l.id].insert(it->second.begin(), it->second.end());
        }
    }
    return true;
}

bool ExtractSolution::prelimCheck(Level& l, set<string> goal_states) {
    set<string>::iterator it;
    for (it = goal_states.begin(); it != goal_states.end(); ++it) {
        if(l.parent_actions.find(*it) == l.parent_actions.end()) return false;
    }
    
    set<string>::iterator it1, it2;
    //state_pair_set = makeAllPossibleStatePairs(goal_states);
    for (it1 = goal_states.begin(); it1 != goal_states.end(); ++it1) {
        for (it2= goal_states.begin(); it2 != goal_states.end(); ++it2) {
            if(*it1 == *it2) continue;
            if(l.state_mutexes.find(makeMutex(*it1, *it2)) != l.state_mutexes.end() || 
               l.state_mutexes.find(makeMutex(*it2, *it1)) != l.state_mutexes.end()) return false;

        }
    }

    return true;
}

bool ExtractSolution::extractSolution(Level& l, set<string> goal_states, map<int, set<Action>>& sol) {

    if(!prelimCheck(l, goal_states)) {
        return false;
    }

    if(cpuTime()-start_time > 100) {
        time_out = true;
        return false;
    }
    set<string>::iterator it;

    map<string, set<Action>> domains_map;
    //cout << "Goal States for Level " << l.id << " : ";
    //printStates(goal_states);
    for (it = goal_states.begin(); it != goal_states.end(); ++it) {
        //printActions(l.parent_actions[*it]);
        domains_map[*it] = l.parent_actions[*it];
        //possible_actions.insert(l.parent_actions[*it].begin(), l.parent_actions[*it].end());    
    }

    if(!AC3(domains_map, l, sol)) {
        //no_goods[l.id].insert(goal_states.begin(), goal_states.end());
        return false;
    }
    map<int, set<Action>>& sol2 = sol;
    return backTracking(domains_map, l, sol2);
}

bool ExtractSolution::removeActionFromDomain(map<string, set<Action>>& domain_map, Action a, queue<string>& q, set<string>& q_set) {
    map<string, set<Action>>::iterator iter;
    for (iter = domain_map.begin(); iter != domain_map.end(); ++iter) {
        if(iter->second.find(a) != iter->second.end()) {
            iter->second.erase(a);
            if(iter->second.size() == 0) return false;
            if(q_set.find(iter->first) != q_set.end()) continue;
            q.push(iter->first);
            q_set.insert(iter->first);
        }
    }
    return true;
}

bool ExtractSolution::Revise(map<string, set<Action>>& domain_map, set<pair<Action, Action>>& action_mutexes, Action a, queue<string>& q, set<string>& q_set) {
    set<pair<Action, Action>>::iterator iter;

    //Action a = *(actions.begin());
    for (iter = action_mutexes.begin(); iter != action_mutexes.end(); ++iter) {
        int found = 0;
        if(iter->first == a) {
            found  = 2;
        } else if(iter->second == a) {
            found = 1;
        }

        if(found) {
            Action b = found==1 ? iter->first : iter->second;
            if(!removeActionFromDomain(domain_map, b, q, q_set)) return false;
        }
    }

    return true;
}

string ExtractSolution::getMRV(map<string, set<Action>> domains_map, map<int, set<Action>>& sol, Level& l) {
    unsigned min = UINT_MAX;
    string min_state("");
    map<string, set<Action>>::iterator it;
    for (it = domains_map.begin(); it != domains_map.end(); ++it) {
        if(it->second.size() == 1) sol[l.id].insert(it->second.begin(), it->second.end());
        //if(it->second.size() == 1) cout << "one size : " << it->second.begin()->name << " for " << it->first << "\n";
        if(it->second.size() > 1 && min > it->second.size()) {
            min = it->second.size();
            min_state = it->first;
        }
    }

    return min_state;
}

bool ExtractSolution::checkSolution(set<Action>& actions) {
   
   if(!actions.size()) return false;
   set<Action>::iterator it;

   for(it = actions.begin(); it != actions.end(); ++it) {
        set<string>::iterator pre_cond_it;
        for (pre_cond_it = it->pre_conds.begin(); pre_cond_it != it->pre_conds.end(); ++pre_cond_it) {
            if(init_states.find(*pre_cond_it) == init_states.end()) return false;
        }
   }
   return true;
}

bool ExtractSolution::backTracking(map<string, set<Action>>& domains_map, Level& l, map<int, set<Action>>& sol) {

    map<int, set<Action>> copy_sol = sol;
    string mrv_state = getMRV(domains_map, copy_sol, l);
    if(!mrv_state.size()) {
        set<string> goal_states;
        //move to previous level
        //cout << "moving to level : " << l.id-1 << "\n";
        if(l.id-1 < 0) {
            if(checkSolution(copy_sol[0])) {
                //cout << "Solution Found\n";
                solutions_map = copy_sol;
                return true;
            } else {
                return false;
            }
        } else {
            map<string, set<Action>>::iterator it;
            //set<Action>::iterator it;
            //set<Action> actions  = solution_map[l.id];
            //for (it = solutions_map[l.id].begin(); it != solutions_map[l.id].end(); ++it) {
            //    goal_states.insert(it->pre_conds.begin(), it->pre_conds.end());
            //}
            for (it = domains_map.begin(); it != domains_map.end(); ++it) {
                goal_states.insert(it->second.begin()->pre_conds.begin(), it->second.begin()->pre_conds.end());
                //cout << "after " << it->second.begin()->name << "\n";printStates(goal_states);
            }
            map<int, set<Action>> temp_sol = sol;
            if(!extractSolution(levels[l.id-1], goal_states, copy_sol)) {
                sol = temp_sol;
                no_goods[l.id-1] = set<string>(goal_states.begin(), goal_states.end());
                return false;
            } else {
                return true;
            }
        }
    }

    set<Action>::iterator it;
    for (it = domains_map[mrv_state].begin(); it != domains_map[mrv_state].end(); ++it) {
        
        map<string, set<Action>> passed_domains = domains_map;
        passed_domains.erase(mrv_state);
        passed_domains[mrv_state] = { *it };
        sol[l.id].insert(*it);
        //cout << "assigning " << it->name << " to level " << l.id << "\n";
        if(AC3(passed_domains, l, sol)) {
            if(!backTracking(passed_domains, l, sol)) {
                //revert solutions_map
                //cout << "reverting " << it->name << " to level " << l.id << "\n";
                sol[l.id].erase(*it);
            } else {
                return true;
            }
        } else {
            sol[l.id].erase(*it);
            //continue;
        }
    }
    return false;
}

void ExtractSolution::printSolution() {
    map<int, set<Action>>::iterator it;
    for (it = solutions_map.begin(); it != solutions_map.end(); ++it) {
        cout << "Level " << it->first << " : ";
        set<Action>::iterator action_it;
        printActions(it->second);
    }
}
