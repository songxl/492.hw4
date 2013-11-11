#include <iostream>
#include <stdio.h>
#include <map>
#include <list>
#include "util.h"

using namespace std;

class ExtractSolution;
void build_action_set(unsigned problem_size, set<Action> &Action_set);
Level expand_graph(Level &l, unsigned problem_size, set<Action> Action_set);
void graphPlan(set<string> states, unsigned problem_size) {
	//set<string> goal_states = makeGoalStates(problem_size);
	Level l; //level 0
	ExtractSolution es;
	set<string> curr_states = states;
	set<string>::iterator it;
	for (it = states.begin(); it!= state.end(); ++it){
		set<Action> emptyset;
		l.parent_actions.insert(pair<string, set<Action>>(*it, emptyset));
	} //Initial planning graph
	es.addLevel(l);
	set<string> goal_states = makeGoalStates(problem_size);
	
	set<Action> Action_set;
	build_action_set(problem_size, Action_set);
	
	while (1){
		if (areGoalFluentsPresent(problem_size, curr_states)){
			if (!areGoalFluentsInMutex(problem_size, curr_states)){
				map<int, set<Action>> dummy_sol;
				if (es.extractSolution(l, goal_states, dummy_sol)){
					return;
					//return dummy_sol;
					//return solution?
				}
			}
		}
		
		Level new_l = expand_graph(l, problem_size, Action_set);
		es.addLevel(new_l);
		l = new_l; //TODO
		curr_states.clear();
		map<string, set<Action>>::iterator state_it;
		for (state_it = new_l.parent_actions.begin(); state_it != new_l.parent_actions.end(); ++ state_it){
			curr_states.insert(state_it->first);
		} //last level all literals
	}
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

Level expand_graph(Level &l, unsigned problem_size, set<Action> Action_set){
	Level new_level;
	set<Action>::iterator action_it;
	set<string>::iterator pre_con_it;
	for (action_it = Action_set.begin(); action_it != Action_set.end(); ++ action_it){
		bool meet = 1;
		for (pre_con_it = action_it->pre_conds.begin(); pre_con_it != action_it->pre_conds.end(); ++pre_con_it){
			//all pre_cond meet?
			if (l->parent_actions.find(*pre_con_it) == l->parent_actions.end()) { meet = 0;}
		}
		if (meet == 1) {
		//this action will be added to the actions map
			new_level.actions.insert(*action_it);
		}
	}
	//add all the effects of the actions to the map
	set<Action>::iterator act_it;
	set<string>::iterator state_it;
	for (act_it = new_level.actions.begin(); act_it != new_level.actions.end(); ++ act_it){
		for (state_it = act_it->effects.begin(); state_it!= act_it->effects.end(); ++state_it){
			if (new_level.parent_actions.find(*state_it) == new_level.parent_actions.end()){
				set<Action> action_set;
				action_set.insert(*act_it);
				new_level.parent_actions.insert(pair<string, set<Action>>(*state_it, action_set));
			}
			else {
				new_level.parent_actions[*state_it].insert(*act_it);
			}
		}
	}
	
	//calculate level mutex
	
	
	
	//calculate state mutex
		
		
		
	}




















}

void build_action_set(unsigned problem_size, set<Action> &Action_set){

	for (int i = 1; i < problem_size; i++){
		int from;
		int to;
		for (int dir = 0; dir <= 1; dir ++){
			from = i + dir;
			to = i + (1 - dir);
		
			//Make the Action PushThru(box, from, to)
			for (int j = 1; j < problem_size; j++){
				Action push_thru;
				string box = string(sprintf("B%d",j));
				push_thru.name = makeAction(box, from, to);
				push_thru.pre_conds.insert(makeState(box, from, 1));
				push_thru.pre_conds.insert(makeState("R", from, 1));
				push_thru.effects.insert(makeState(box, to, 1));
				push_thru.effects.insert(makeState(box, from, 0));	
				push_thru.effects.insert(makeState("R", to, 1));	
				push_thru.effects.insert(makeState("R", from, 0));
				Action_set.insert(push_thru);
			}
		
			//GoThru(from, to)
				Action go_thru;
				go_thru.name = makeAction(from, to);
				go_thru.pre_conds.insert(makeState("R", from, 1));
				go_thru.effects.insert(makeState("R",to, 1));
				go_thru.effects.insert(makeState("R",from, 0));
				Action_set.insert(go_thru);
			
		}
		//Persist(state)
		Action persist;
		//box
		for (int j = 1; j < problem_size; j++){
			string box = string(sprintf("B%d",j));
			for (int g = 0; g <= 1; g++){
				persist.name = makeAction(makeState(box, i, g));
				persist.pre_cons.insert(makeState(box, i, g));
				persist.effects.insert(makeState(box, i, g));
				Action_set.insert(persist);
			}
		}
		//robot
		for (int g = 0; g <= 1; g++){
				persist.name = makeAction(makeState("R", i, g));
				persist.pre_cons.insert(makeState("R", i, g));
				persist.effects.insert(makeState("R", i, g));
				Action_set.insert(persist);
		}
	
		if (i == problem_size - 1){
			for (int j = 1; j < problem_size; j++){
				string box = string(sprintf("B%d",j));
				for (int g = 0; g <= 1; g++){
					persist.name = makeAction(makeState(box, i+1, g));
					persist.pre_cons.insert(makeState(box, i+1, g));
					persist.effects.insert(makeState(box, i+1, g));
					Action_set.insert(persist);
				}
			}
			for (int g = 0; g <= 1; g++){
				persist.name = makeAction(makeState("R", i+1, g));
				persist.pre_cons.insert(makeState("R", i+1, g));
				persist.effects.insert(makeState("R", i+1, g));
				Action_set.insert(persist);
			}
		}	
	}


};















