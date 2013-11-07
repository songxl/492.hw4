#include <iostream>
#include <fstream>
#include <stdio.h>
#include <queue>
#include <map>
#include <set>
#include <limits.h>

using namespace std;

/*
An Action has pre_conds and effects as sets of strings
name of an Action is like : "GoThrough(B1,1,2)" which can be returned by makeAction() functions
*/
struct Action {
    set<string> pre_conds;
    set<string> effects;
    string name;
    Action(set<string> pre_conds, set<string> effects) : pre_conds(pre_conds), effects(effects) {};
    Action(set<string> pre_conds, set<string> effects, string name) : pre_conds(pre_conds), effects(effects), name(name) {};
};

/*
Each Level Contains the actions and also the states of effects of current level in parent_actions map in which value of each row is the 
set of actions that have effect as this state
*/
struct Level {
    int id; //unique id
    map <string, set<Action>> parent_actions; // map from states to actions that have this state as the effect
    set<Action> actions; //set of actions being executed at this level
    set<pair<string, string>> state_mutexes; // set of pairs of state/literal mutexes at this level
    set<pair<Action, Action>> action_mutexes; //set of pairs of action mutexes at this level
    Level(map <string, set<Action>> parent_actions, set<Action> actions) : parent_actions(parent_actions), actions(actions) {};
};

/*
use this class to start the return a task from tasks.txt
Ex : 
GraphPlan gp("tasks.txt");
gp.nextTask(size); // size if the number of rooms in the problem
*/
class GraphPlan {
    ifstream ifile;
        public:
            GraphPlan(string file) {
                ifile.open (file.c_str (), ios::in);
            }

            set<string> nextTask(unsigned& problem_size);
};

/*
This class is used for extracting a solution at a specific level.
Example usage : 
//define an object
//cpuTime() returns the cur time. It helps in timing out the task is it takes too long.
ExtractSolution es(problem_size, init_states, cpuTime()); 

// add a level to extract solution
es.addLevel(l); //where l is the level

//call the backtracking with AC3 on a given level l and goal states of this problem
solution exists = es.extractSolution(l, goal_states);

Note : You only have to call ExtractSolution::extractSolution() on an object of ExtractSolution, the rest of CSP backtracking algo 
       is already implemented. For details on that look at ExtractSolution.cpp

       bool extractSolution(Level& l, set<string> goal_states,  map<int, set<Action>>& sol); -> in this function the last parameter
			 is a dummy map, its not the actual solution map. So just pass a dummy map as the last parameter. For exampple:
			 map<int, set<Actions>> dummy_sol;
			 es.extractSolution(l, goal_states, dummy_map)
*/
class ExtractSolution {
    unsigned problem_size;
    set<string> init_states;
    map<int, set<Action>> solutions_map;
    vector<Level> levels;
    map<int, set<string>> no_goods;
    double start_time;

    public:
    bool time_out;
    ExtractSolution(unsigned size, set<string> inits, double curr_time) {
        problem_size = size;
        init_states = inits;
        start_time = curr_time;
        time_out = false;
    }

    void addLevel(Level& l) {
        l.id = levels.size();
        levels.push_back(l);
    }

    bool prelimCheck(Level& l, set<string> goal_states);
    bool extractSolution(Level& l, set<string> goal_states,  map<int, set<Action>>& sol);
    bool Revise(map<string, set<Action>>& domain_map, set<pair<Action, Action>>& action_mutexes, Action a, queue<string>& q, set<string>& q_set);
    bool AC3(map<string, set<Action>>& domains_map, Level& l,  map<int, set<Action>> sol);
    bool removeActionFromDomain(map<string, set<Action>>& domain_map, Action a, queue<string>& q, set<string>& q_set);
    bool backTracking(map<string, set<Action>>& domains_map, Level& l, map<int, set<Action>>& sol);
    string getMRV(map<string, set<Action>> domains_map, map<int, set<Action>>& sol, Level& l);
    bool checkSolution(set<Action>& actions);
    void printSolution();
    map<int, set<Action>> getSolution() { return solutions_map;}
};

/* ! operator overloaded for strings.
   Ex : string s  = "B1:2:1";
        string s2 = !s; // s2 = "B1:2:0"
*/
string operator! (string state);

/*
   == operator overloaded
   == overloading works on just the name of Action

   < operator overloaded
   <  overloading works on just the name of Action
*/
bool operator == (const Action& a, const Action& b);
bool operator < (const Action& a, const Action& b);

/*
adds the action "a" to all the entry of all the effects in parent_actions
Ex : 
map<string, set<Action>> parent_actions;
Action a(pre_conds, effects, name);
addToParentActions(parent_actions, effects, a);
*/
void addToParentActions(map <string, set<Action>>& parent_actions, set<string> effects, Action& a);

/*
returns the state in string format.
For instance In("R", 1) can be written as "R:1:1"
and ~In("B2", 3) can be written as "B2:3:0"

Ex : 
makeState("R", 1, true) wille return "R:1:1"
*/
string makeState(string obj, unsigned room, bool present);

/*
returns the strng of the action for GoThrough
Ex : makeAction(1,2) will return "GoThrough(1,2)"
The value returned by makeAction is used as the name for that action
*/
string makeAction(int from, int to);

/*
returns the strng of the action for PushThrough
Ex : makeAction("B1", 1,2) will return "PushThrough(B1,1,2)"
The value returned by makeAction is used as the name for that action
*/
string makeAction(string box, int from, int to);

/*
returns the string of the persistant action for a state
Ex : makeAction("B1:1:1") will return "Pers(B1:1:1)"
The value returned by makeAction is used as the name for that action
*/
string makeAction(string state);

/*
adds a pair <Action, Action> (a,b) and pair<Action, Action>(b,a) to action_mutexes
*/
void addToActionMutexes(const Action& a, const Action& b, set<pair<Action, Action>>& action_mutexes);

/*
adds a pair <string, string> (a,b) and pair<string, string>(b,a) to state_mutexes
*/
void addToStateMutexes(string a, string b, set<pair<string, string>>& state_mutexes);

/*
prints all states in the passed set
*/
void printStates(set<string>& states);

/*
prints all actions in passed set
*/
void printActions(set<Action>& actions);

/*
returns the initial fluents/states from the task given in tasks.txt.
Note : This won't be needed for implementation. Its an internal helper function
*/
set<string> getInitFluents(string line);

/*
checks for state1 and state2 if they can come from the same parent action.
If so, they shouln't be added to state mutexes
*/
bool checkSameParentEffect(set<Action>& pa, string state1, string state2);

/*
returns true if two states can occur from the same set of actions either pa1 or pa2

This actually calls checkSameParentEffect(set<Action>& pa, string state1, string state2);
on each set of actions pa1 and pa2

This function would be useful if two states are being checked for stateMutex and if the two states are actually
from the same action in any of the set of parent actions of state1 or state2 then we dont wanna have state mutex between them
*/
bool checkSameParentActionEffects(set<Action>& pa1, set<Action>& pa2, string state1, string state2);

/*
returns a set of all possible pairs of actions in pa1 and pa2
*/
set<pair<Action, Action>> makeAllPossibleActionPairs(set<Action>& pa1, set<Action>& pa2);

/*
returns a set of all possible states pairs in parent_actions
*/
set<pair<string, string>> makeAllPossibleStatePairs(map<string, set<Action>>& parent_actions);

/*
for the given problem size, returns the final goal state
*/
set<string> makeGoalStates(unsigned problem_size);

/*
checks if all the goal fluents are present in states for a given problem size
*/
bool areGoalFluentsPresent(unsigned problem_size, set<string> states);

/*
checks if any of the goal fluents are mutex with each other 
*/
bool areGoalFluentsInMutex(unsigned problem_size, set<pair<string, string>> state_mutexes);

/*
returns a pair of <T,T>(a,b)
*/
template <class T>
pair<T, T> makeMutex(const T a, const T b); 

// returns the current cpu  time, used by extractSolution 
double cpuTime(void);
