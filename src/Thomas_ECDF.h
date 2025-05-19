// Original Author: Vinh Nguyen

//Currently Reviewing: Thomas Carroll

#ifndef ECDF_H
#define ECDF_H

#include <algorithm>
#include <vector>
#include <cmath>
#include <limits.h>

#include "generate_task_set.h"

//------------ MOD FUNCTION ------------------

double MOD(double t, double Ti) {
    double res = t - floor(t / Ti) * Ti;
    
    return res;
}

//---------- PROPOSITION 1 FUNCTIONS - EDF schedulability in LC-------------

double dbfLi(int time, int Ti, double DiL, double CiL) { //Equation (1)
    double difference = double(time) - double(DiL);
    double divRes = difference / Ti;
    double RHS = (floor(divRes) +  1) * CiL;
    double res = max(0.0, RHS);

    return res;
}

bool Proposition1(int t, TaskSet taskSet) {
    double sum = 0;

    // for (int i = 0; i < candidates.size(); i++) {
    //     Task task = candidates[i];
    //     sum += dbfLi(t, task.T, task.tight_D, task.C_LO);
    // }
    for (const auto& pair : taskSet.get_task_set()) {
        const Task& task = pair.second;
        sum += dbfLi(t, task.T, task.tight_D, task.C_LO);
    }

    if (sum > t) {
        return false;
    }
    
    
    return true;
}

double dbfHi_l4(int t1, int t2, double CiH, double Di, int Ti) {
    double RHS = (floor((t2 - t1 - Di) / Ti) + 1) * CiH;

    return max(double(0), RHS);
}

double dbfUNi_l2(int t1, int t2, double CiL, double DiL, double Ti) {
    double res = 0;
    double modRes = MOD(t1, Ti);
    double divRes = t1 / Ti;
    double clause2LHS = (floor(divRes)) * Ti + DiL;

    if (DiL > modRes && clause2LHS <= t2)
        res = min(double(CiL), modRes);

    return res;
}

bool isCase1(int t1, int t2, Task task) {
    return (task.L == Level::HI && ((t2 - t1) <= (task.D - task.tight_D)));
}

bool isCase2(int t1, int t2, Task task) {
    return (task.L == Level::HI && ((t2 - t1) > (task.D - task.tight_D)) &&
            ((task.D - task.tight_D) < MOD(t2 - t1, task.T)) && // case 2 lemma 4 pass
            (MOD(t2 - t1, task.T) < task.D) &&      
            ((floor((t2 - t1) / task.T) * task.T + task.D) <= t2)); // 
}

bool isCase3(int t1, int t2, Task task) {
    return (task.L == Level::HI && ((t2 - t1) > (task.D - task.tight_D)) &&
            !((task.D - task.tight_D) < MOD(t2 - t1, task.T) && // case 3 lemma 4 fail
                (MOD(t2 - t1, task.T) < task.D) &&      
                ((floor((t2 - t1) / task.T) * task.T + task.D) <= t2))  &&
            
                !((task.tight_D > MOD(t1, task.T)) && // case 3 lemma 2 fail I personally don't know why this is here...
                ((floor(t1 / task.T) / task.T + task.tight_D) <= t2)));
}

double dbfUN_l6(int t1, int t2, TaskSet& taskSet) {
    double res = 0;
    double dbfUNi_Sum = 0;
    double max_DiL = 0;
    map<int, Task> taskSetMap = taskSet.get_task_set();

    for (const auto& pair : taskSetMap) {
        const Task& task = pair.second;
        
        if (task.L == Level::LO || isCase1(t1, t2, task)) { //or HC tasks and case 1
            dbfUNi_Sum += dbfUNi_l2(t1, t2, task.C_LO,  task.tight_D, task.T);

            if (max_DiL < task.tight_D) {
                max_DiL = task.tight_D;
            }
        }
    }

    res = min(max_DiL, dbfUNi_Sum);

    return res;
}

double dbfL1_l7(int t1, int t2, TaskSet& taskSet) {
    double res = 0;
    double dbfLi_Sum = 0;
    double dbfUN_Res = dbfUN_l6(t1, t2, taskSet);

    map<int, Task> taskSetMap = taskSet.get_task_set();
    
    for (const auto& pair : taskSetMap) {
        const Task& task = pair.second;
        
        if (task.L == Level::LO || isCase1(t1, t2, task)) { //or HC tasks and case 1
            dbfLi_Sum += dbfLi(t1, task.T, task.tight_D, task.C_LO);
        }
    }

    return dbfLi_Sum + dbfUN_Res;
}

double dbfLi_l4(int t1, int t2, double CiL, double Di, int Ti) {
    double RHS = (floor((t2 - Di) / Ti) - floor ((t2 - t1 - Di) / Ti) - 1);

    return max(double(0), RHS) * CiL;
}

double CO_prop2(int time, double CiL, double Di, double DiL, int Ti) {
    double RHS = MOD(time, Ti) - (Di - DiL);

    return min(double(CiL), RHS);
}

double dbfL2_l8(int t1, int t2, TaskSet& taskSet) {
    double res = 0;
    map<int, Task> taskSetMap = taskSet.get_task_set();
    
    for (const auto& pair : taskSetMap) {
        const Task& task = pair.second;
        
        if (isCase2(t1, t2, task)) {
            double cal = dbfLi_l4(t1, t2, task.C_LO, task.D, task.T)\
                        + task.C_LO\
                        + CO_prop2(t2 - t1, task.C_LO, task.D, task.tight_D, task.T);
            res += cal;
        }
    }

    return res;
}

double dbfL3_l9(int t1, int t2, TaskSet& taskSet) {
    double res = 0;
    map<int, Task> taskSetMap = taskSet.get_task_set();
    
    for (const auto& pair : taskSetMap) {
        const Task& task = pair.second;
        
        if (isCase3(t1, t2, task)) {
            double cal = dbfLi_l4(t1, t2, task.C_LO, task.D, task.T)
                        + task.C_LO;
            res += cal;
        }
    }

    return res;
}

double Theorem2_LHS(int t1, int t2, TaskSet& taskSet) {
    double dbfL1_Res = dbfL1_l7(t1, t2, taskSet);//Lemma 7
    double dbfL2_Res = dbfL2_l8(t1, t2, taskSet);//Lemma 8
    double dbfL3_Res = dbfL3_l9(t1, t2, taskSet);//Lemma 9
    // maybe add assert here for the conditions?
    double dbfHi_Res = 0; //dbfHi_l4(t1, t2, CiH[0], Di[0], Ti[0]);
    double CO_Res = 0; //CO(t2 - t1, CiL[0], Di[0], DiL[0], Ti[0]);

    map<int, Task> taskSetMap = taskSet.get_task_set();
    
    for (const auto& pair : taskSetMap) {
        const Task& task = pair.second;
        
        if (task.L == Level::HI) {
            if ((t2 - t1) > (task.D - task.tight_D)) {
                if  (isCase2(t1, t2, task)) { 

                    double CO_Cal = CO_prop2(t2 - t1, task.C_LO, task.D, task.tight_D, task.T)
                                + (task.C_HI - task.C_LO); 
                    CO_Res += CO_Cal;
                    dbfHi_Res += dbfHi_l4(t1, t2, task.C_HI, task.D, task.T);// lemma 4
                }

                else if (isCase3(t1, t2, task)) { 
                    dbfHi_Res += dbfHi_l4(t1, t2, task.C_HI, task.D, task.T);
                }
            }
        }
    }

    double sumDBFs = dbfL1_Res + dbfL2_Res + dbfL3_Res;
    double min_t1 = min(double(t1), sumDBFs);
    double res = min_t1 + dbfHi_Res + CO_Res;

    return res;
}

bool Theorem2(int t1, int t2, TaskSet& taskSet) {
    double res = Theorem2_LHS(t1, t2, taskSet);

    return res <= t2;
} 

int findCandiate(vector<Task> candidates, int t1, int t2, TaskSet& taskSet) {//line 22
    double Theorem2_LHS_Res = Theorem2_LHS(t1, t2, taskSet);// we have to calculate theorem 2 twice this way can maybe do it once and save the results?
    double DEM = Theorem2_LHS_Res - t2;//line 23
    int res = -1;// line 24
    double DIFF = 0, DEC = INFINITY;// line 24

    for (int i = 0; i < candidates.size(); i++) {// line 25
        if (isCase2(t1, t2, candidates[i]) &&

            ((candidates[i].C_HI - candidates[i].C_LO) >= DEM)) {  
            temp_val = (MOD(t2 - t1, candidates[i].T) - (candidates[i].D - candidates[i].tight_D))
            if (temp_val < DEC) {
                DEC = temp_val
                res = i;
                DIFF = candidates[i].C_HI - candidates[i].C_LO;
            }
            else if (temp_val == DEC) {
                if (candidates[i].C_HI - candidates[i].C_LO > DIFF) {
                    res = i;
                    DIFF = candidates[i].C_HI - candidates[i].C_LO;
                }
            }
        }    
    }

    return res;
}

void removeCandidateByIndex(std::vector<Task>& candidates, int index) {
    if (index >= 0 && index < candidates.size()) {
        candidates.erase(candidates.begin() + index);
    }
}

string ECDF(TaskSet& taskSet) {
    int i = -1, tMax = taskSet.get_t_max(), min_diff = INT_MAX; //line 4 support
    vector<Task> candidates;
    map<int, Task> taskSetMap = taskSet.get_task_set();
    
    for (const auto& pair : taskSetMap) { 
        const Task& task = pair.second;
        
        if (task.L == Level::HI) {
            candidates.push_back(task);
            min_diff = min(min_diff, (task.D - task.tight_D));// for line 11 2nd condition Looks like missing a -1 here?
        }
    }
    
    while (true) {// line 2
        bool feasible = true; //line 3

        for (int t = 0; t <= tMax; t++) { // line 4
            if (!Proposition1(t, taskSet)) {
                if (i == -1) {
                    return "Failure";
                }
                
                candidates[i].tight_D += 1;
                removeCandidateByIndex(candidates, i);
                
                i = -1; // assign undefined line 8
                break;
            }
        }

        for (int t2 = 0; t2 <= tMax; t2++) {// 1st condition line 11
            for (int t1 = 0; t1 < t2 - min_diff-1; t1++) { //2nd condition line 11 added previously //missing -1
                if (!Theorem2(t1, t2, taskSet)) { // line 12
                    if (t1 == 0 || candidates.empty()) { // line 13
                        return "Failure";
                    }

                    i = findCandiate(candidates, t1, t2, taskSet);// line 14
                    candidates[i].tight_D -= 1;

                    if (candidates[i].tight_D - 1 < candidates[i].C_LO) {
                        removeCandidateByIndex(candidates, i);
                    }

                    feasible = false;
                    break;
                }
            }
        }
        if (feasible) {
            return "Success";
        }
    }

    return "Unknown"; // This line should never be reached
}

#endif
