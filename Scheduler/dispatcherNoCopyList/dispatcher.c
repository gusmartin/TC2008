/*
 * Copyright (c) 2017 
 * 
 * File name: Dispatcher.h
 *
 * Author:  Gustavo Martin
 *
 * Purpose: Calculate different types of dispatcher algorithms, 
 *			also able to do process handling like creating and sorting
 *
 * References:
 *          Algorithms based on the ones we saw in class
 *			Code based on our previous knowledge
 *
 * Restrictions:
 *          For the functions that require a char *criteria parameter, 
 *			if you leave it blank or type something else it will default to priority 
 *
 * Revision history:
 *
 *          Sep  20  -- File created, added the CreateProcess, SortProcessList, PrintProcessList and DestroyList functions
 *
 *          Sep  21  -- Preemptive function created, modified CreateProcess to support the modified struct od Process
 *
 *          Sep  24  -- Preemtive function now supports having the processor idle
 *
 *          Sep  26  -- Created RoundRobin, FirstCome and NonPreemptive functions
 *
 *
 * Error handling:
 *          None
 *			(The only error that could happen is if the timePassed variable reaches INT_MAX)
 *
 * Notes:
 *          Code could be optimized by implementing g_list_copy_deep() in the preemptive, nonpreemptive and roundrobing functions
 *
 * $Id$
 */


#include <glib.h>				 /* Used for linked lists and sorting */
#include <stdlib.h>                     /* Used for malloc definition */
#include <stdio.h>                                /* Used for printf */
#include <string.h>                                /* Used for strcmp */


//Definition of the structure: Process
typedef struct {
	
	int pid;
	int arriveTime;
	int burst;
	int priority;
	
	int burstLeft;   	/* This is used as a copy of burst to be able to alter its value without losing its initial value*/
	int bgPriority;   	/* This is used to identify terminated processes to new processes*/
	
}Process;

// Create a new process for the list: allocate memory and assign values
Process *newProcess(int pid, int arriveTime, int burst, int priority, int burstLeft)
{
    Process *process = malloc(sizeof(Process));
    //Process *process = g_new(Process, 1);
    
    process->pid = pid;
    process->arriveTime = arriveTime;
    process->burst = burst;
    process->priority = priority;
    process->burstLeft = burstLeft;
    process->bgPriority = 0;
    
    return(process);
}


  //								 //
 // Compare functions for sorting   //
//								   //

// compare by time of arrival, if same time by pid
gint compareArrival(gconstpointer *a, gconstpointer *b){
	
	int compare = ((Process*)a)->bgPriority - ((Process*)b)->bgPriority; // first we sort by backgroundPriority because if they are not 0
																		//  we send them to the last of the list
	
	if(compare != 0) 		
		return compare;
	
	compare = ((Process*)a)->arriveTime - ((Process*)b)->arriveTime;
	
	if(compare != 0) 		// if arriveTimes are diferent return result
		return compare;
	
	else					// if not, compare by pid and return
		return ((Process*)a)->pid - ((Process*)b)->pid;
}

// compare by burst lenght, if same time by pid
gint compareBurst(gconstpointer *a, gconstpointer *b){
	
	int compare = ((Process*)a)->burstLeft - ((Process*)b)->burstLeft;
	
	if(compare != 0)
		return compare; 	// if bursts are diferent return result
		
	//if burst is the same, sort by arriveTime
	compare = ((Process*)a)->arriveTime - ((Process*)b)->arriveTime;
	
	if(compare != 0 )
		return compare;

	// if arriveTime is the same, sort by ID
	else
		return ((Process*)a)->pid - ((Process*)b)->pid;
}

// compare by priority, if same time by pid
gint comparePriority(gconstpointer *a, gconstpointer *b){
	
	int compare = ((Process*)a)->bgPriority - ((Process*)b)->bgPriority;
	
	if(compare != 0)
		return compare;		// first Check if bgPriority is greater than 0, if it is the process has already been processed */
	
	compare = ((Process*)a)->priority - ((Process*)b)->priority;
	
	if(compare != 0)
		return compare;		// if priorities are diferent return result
			
	//if priority is the same, sort by arriveTime
	compare = ((Process*)a)->arriveTime - ((Process*)b)->arriveTime;
	
	if(compare != 0 )
		return compare;
	
	// if arriveTime is the same, sort by ID
	else
		return ((Process*)a)->pid - ((Process*)b)->pid;
}


/***********************************************************************
 *                          List Handling functions                    *
 **********************************************************************/
 

// Creates a process and appends it to the list, returns new list 
GList* CreateProcess(GList *processList, int pid, int arriveTime, int burst, int priority){
 	
 	Process *process = newProcess(pid, arriveTime, burst, priority, burst);
 	
 	processList  = g_list_append(processList, process);
 			
 	return processList;
}

//Sorts a list by arrivalTime using compareArrival
//Function currently NOT used
GList* SortProcessList(GList* list){
	
	
	list = g_list_sort(list, (GCompareFunc)compareArrival);
	
	return list;
}

//Prints the list 
void PrintProcessList(GList* processList){
	
	GList *current = NULL;
	
	
	for (current = processList; current; current = current->next) {  // stops until current is NULL 
		
	  	printf("%d %d %d %d\n", ((Process*)current->data)->pid, ((Process*)current->data)->arriveTime, 
		  						((Process*)current->data)->burst, ((Process*)current->data)->priority  );
		  
				
	}
}

// Dealocate memory, function needs to be called after handling the list and before terminating
void DestroyList(GList *processList){
	
	 g_list_foreach(processList, (GFunc)g_free, NULL); 	// Desroy each process
	 g_list_free(processList); 							//Destroy the list	
	
	
}

// Sets the burstLeft = burst for all processes in the list
void renewBurst(GList *processList){
	
	GList *current = NULL;
	
	for (current = processList; current; current = current->next) {  // stops until current is NULL 
		
	  	((Process*)current->data)->burstLeft = ((Process*)current->data)->burst; // copy the burst to the burstLeft  			
	}
}

// Sets the bgPriority = 0 for all processes in the list
void setBgPriority(GList *processList){
	
	GList *current = NULL;
	
	for (current = processList; current; current = current->next) {  // stops until current is NULL 
		
	  	((Process*)current->data)->bgPriority = 0; // set bgPriority to 0			
	}
}


  //								 //
 // DISPATCHER ALGORITHM FUNCTIONS  //
//								   //


//This function is used for the priority and SJF algorithms
//It needs the processList header and a string to diferentiate betweeen the diferent algorithms
void Preemptive(GList* list, char *criteria){
	
	//for traversing the list
	GList* current;
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(list);
	
	//used to keep the processor idle until a process arrives
	int found;
	
	int i = 0;
	
	renewBurst(list); // Refresh the burstLeft variable so we can manipulate it
				
	if(!strcmp(criteria, "priority")){	
		setBgPriority(list);			 // If we are using priority, we use this variable to know when a process is done, 
										//so first we must set all the values to 0
									   // I believe this is cleaner than having 2 functions for priority and SJF, but I might be being lazy								   
	}
								
	while(i < size){
			
		//restart found, at the beginning we don´t know if there are any proceses waiting
		found = 0;
		
		
		//Sort the list depending on the chosen criteria
		if(!strcmp(criteria, "burst"))
			list = g_list_sort(list, (GCompareFunc)compareBurst);
		
		else
			list = g_list_sort(list, (GCompareFunc)comparePriority);	
		
			
		
		//traverse the list to find the next process
		for(current = list; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed && ((Process*)current->data)->burstLeft != INT_MAX){ //use the first process of the list that has arrived
																													   // also check if != to INT MAX because that means the process has been done	
				found = 1;	//if process is found stop searching
				break;
			}	
		}
		
		//everycycle increase by one
		timePassed ++;
		
		if(found){
			
			((Process*)current->data)->burstLeft -= 1; 		//reduce the burst by one
			
						
			if(((Process*)current->data)->burstLeft == 0){
				
				((Process*)current->data)->bgPriority = INT_MAX; // send the process to the end of the list
				((Process*)current->data)->burstLeft = INT_MAX; // send the process to the end of the list
															   //we don´t eliminate the process because we still need the list for other dispatcher algorithms
				
				int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; //get the wait time to the process
				
				totalWaitTime += waitTime;						//add the process waitTime to the total waitTime
				
				i++;   										//increase number of finished process
			}	
		}
	}
	
	//calculate average wait time of algorithm
	totalWaitTime /= size;

	printf("%f\n\n", totalWaitTime);	
}

void RoundRobin(GList* list, int quantum){
	
	//for traversing the list
	GList* current;
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(list);
	
	//used to keep the processor idle until a process arrives
	int found;
	
	int i = 0 , j;
	
	renewBurst(list); // Refresh the burstLeft variable so we can manipulate it
	
	setBgPriority(list);  // we use this to send the finished processes to the end of the list
					
	while(i < size){
			
		//restart found, at the beginning we don´t know if there are any proceses waiting
		found = 0;
		
		list = g_list_sort(list, (GCompareFunc)compareArrival);
			
		//traverse the list to find the next process
		for(current = list; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed && ((Process*)current->data)->bgPriority != INT_MAX){ //use the first process of the list that has arrived
																														// also check if != to INT MAX because that means the process has been done		
				found = 1;	//if process is found stop searching
				break;
			}	
		}
		
		if(found){
						
			for(j = 0; j < quantum; j++){
				
				timePassed ++; 									// increase time passed by the quantum
				((Process*)current->data)->burstLeft -= 1; 				//reduce the burst by the quantum
				
				if(((Process*)current->data)->burstLeft == 0)
					break;											// make sure that the process stops when it reaches 0
			}
			
			if(((Process*)current->data)->burstLeft == 0){
				
				int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; //get the wait time to the process
				
				totalWaitTime += waitTime;						//add the process waitTime to the total waitTime
				
				i++;   										//increase number of finished process
				
				((Process*)current->data)->bgPriority = INT_MAX; // send the process to the end of the list
				
			}	
			else  	//send the process from first to last
				((Process*)current->data)->bgPriority += 1; // this sends the process to the end of the list because we are using bgPriority as a sorting criteria
		}
		
		else
			timePassed++;						//if there´s no process in the queue, increase time by 1

	}
		
	//calculate average wait time of algorithm
	totalWaitTime /= size;

	printf("%f\n\n", totalWaitTime);
	
}

void FirstCome(GList* list){
	
	//for traversing the list
	GList* current;
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(list);
	
	int i = 0;
	
	setBgPriority(list);  // change the backgorund priority to 0 
		
	//sort the list one time only
	list = g_list_sort(list, (GCompareFunc)compareArrival);
	
	current = list;
					
	while(i < size){
											
		if(((Process*)current->data)->arriveTime > timePassed){ //use the first process of the list that has arrived
																												
			timePassed++; // if a process has not arrived, increase the time and recheck
		}
		
		else{
			timePassed += ((Process*)current->data)->burst;  // increase timePassed by burst amount
						
			int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; //get the wait time to the process
			
				
			totalWaitTime += waitTime;						//add the process waitTime to the total waitTime
				
			i++;   										//increase number of finished process
				
			current = current->next; //then, go to the next process	
		}
	}
		
	//calculate average wait time of algorithm
	totalWaitTime /= size;

	printf("%f\n\n", totalWaitTime);	
}

//This function is used for the priority and SJF  NonPreemptive algorithms
//It needs the processList header and a string to diferentiate betweeen the diferent algorithms
void NonPreemptive(GList* list, char *criteria){
	
	//for traversing the list
	GList* current;
		
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(list);
	
	int i = 0, found;
	
	//Just in case
	renewBurst(list); 
	setBgPriority(list);  
								
	while(i < size){
		
		
		//Sort the list depending on the chosen criteria
		if(!strcmp(criteria, "burst"))
			list = g_list_sort(list, (GCompareFunc)compareBurst);
		
		else
			list = g_list_sort(list, (GCompareFunc)comparePriority);
			
		
		found = 0;
		
		//traverse the list to find the next process
		for(current = list; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed && ((Process*)current->data)->burstLeft != INT_MAX){ //use the first process of the list that has arrived
																													   // also check if != to INT MAX because that means the process has been done	
				found = 1;	//if process is found stop searching
				break;
			}	
		}
		
		if(found){
			
			timePassed += ((Process*)current->data)->burst;  // increase timePassed by burst amount
						
			int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; //get the wait time to the process
				
			totalWaitTime += waitTime;						//add the process waitTime to the total waitTime
				
			i++;   										//increase number of finished process
					
			((Process*)current->data)->burstLeft = INT_MAX;  //send the process to the end of the list
		}
		
		else
			timePassed++; // if a process has not arrived, increase the time and recheck
	}
		
	//calculate average wait time of algorithm
	totalWaitTime /= size;

	printf("%f\n\n", totalWaitTime);
}

