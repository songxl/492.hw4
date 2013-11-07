#include <iostream>
#include <stdio.h>
#include <map>
#include <list>
#include "util.h"

using namespace std;

class ExtractSolution;

void graphPlan(set<string> states, unsigned problem_size) {
}

int main() {
    GraphPlan gp("tasks.txt");
    unsigned problem_size;
    set<string> states = gp.nextTask(problem_size);
    //graphPlan(states, problem_size);
    unsigned task = 1;
    while(states.size() != 0) {
        cout << "Task " << task++ << "\n";
        set<string> goal_states = makeGoalStates(problem_size);
        printStates(states);
        cout << "=====>";
        printStates(goal_states);
        graphPlan(states, problem_size);
        states = gp.nextTask(problem_size);
        //break;
    }
}
