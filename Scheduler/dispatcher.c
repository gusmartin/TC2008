/*
 * Copyright (c) 2017 
 * 
 * File name: Dispatcher.c
 *
 * Author:  Gustavo Martin 	A01362246
 *
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
 *          Oct  1  --  Optimized the code, now we copy the list to be able to handle the processes in a more elegant and efficient way
 *
 *
 * Error handling:
 *          None
 *			(The only error that could happen is if the timePassed variable reaches INT_MAX)
 *
 * Notes:
 *          We could modify the RoundRobin function for moving a process from its position to the last instead of eliminating and appending to the list
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
	
	int burstLeft;   		/* This is used as a copy of burst to be able to alter its value without losing its initial value*/
	int firstArriveTime; 	/* This is used in the roundRobin Algorithm to keep track of the true arrivalTime since we change the other variable*/
		
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
    
    process->burstLeft = burst;
    process->firstArriveTime = arriveTime;
    
    return(process);
}

  //								 //
 // Compare functions for sorting   //
//								   //

// compare by time of arrival, if same time by pid
gint compareArrival(gconstpointer *a, gconstpointer *b){
	
	int compare = ((Process*)a)->arriveTime - ((Process*)b)->arriveTime;
	
	if(compare != 0) 		// if arriveTimes are different return result
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
	
	int compare = ((Process*)a)->priority - ((Process*)b)->priority;
	
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

// Create a copy of the list given
//SRC  is the data to copy
//data is something to pass to the function, can be nullable
GCopyFunc copyList(gconstpointer src, gpointer data){
	
    Process *process = malloc(sizeof(Process)); 

    process->pid = ((Process*)src)->pid;
    process->arriveTime = ((Process*)src)->arriveTime;
    process->burst = ((Process*)src)->burst;
    process->priority = ((Process*)src)->priority;
    
    process->burstLeft = ((Process*)src)->burstLeft;
    process->firstArriveTime = ((Process*)src)->firstArriveTime;
    
    return (gconstpointer)process;
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
		  						((Process*)current->data)->burst, ((Process*)current->data)->priority);
								
		  
				
	}
}

// Dealocate memory, function needs to be called after handling the list and before terminating
void DestroyList(GList *processList){
	
	 g_list_foreach(processList, (GFunc)g_free, NULL); 	// Desroy each process
	 g_list_free(processList); 							//Destroy the list	
	
	
}


  //								 //
 // DISPATCHER ALGORITHM FUNCTIONS  //
//								   //


//This function is used for the priority and SJF algorithms
//It needs the processList header and a string to diferentiate betweeen the diferent algorithms
void Preemptive(GList* list, char *criteria){
	
	//for traversing the list
	GList* current;
	
	GList* copy = g_list_copy_deep(list, (GCopyFunc)copyList, NULL);
	
	//temporarily store a Process
	Process* temp;
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(copy);
	
	//used to keep the processor idle until a process arrives
	int found;
	
	//Sort the list depending on the chosen criteria
	if(!strcmp(criteria, "priority"))
		copy = g_list_sort(copy, (GCompareFunc)comparePriority);
	
	//while there are processes to process				
	while(copy){
			
		//restart found, at the beginning we don´t know if there are any proceses waiting
		found = 0;
		
		//Sort the list depending on the chosen criteria, since processes bursts change every cycle we need to re-sort
		if(!strcmp(criteria, "burst"))
			copy = g_list_sort(copy, (GCompareFunc)compareBurst);
				
		//traverse the list to find the next process
		for(current = copy; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed){ 			//use the first process of the list that has arrived
																												
				found = 1;														//if process is found stop searching
				break;
			}	
		}
		
		//everycycle increase by one
		timePassed ++;
		
		if(found){
			
			((Process*)current->data)->burstLeft -= 1; 							//reduce the burst by one
						
			if(((Process*)current->data)->burstLeft == 0){						//if process is done
				
				int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; 	//get the wait time to the process
				
				//printf("waitime of %d is %d\n", ((Process*)current->data)->pid, waitTime );
				
				totalWaitTime += waitTime;										//add the process waitTime to the total waitTime
				
				copy = g_list_remove(copy, ((Process*)current->data));			// remove process from the list 
			}
			
			/*else{  	//send the process to the end of the list
				
				temp = ((Process*)current->data);								//store a process temporarily
					
				copy = g_list_remove(copy, ((Process*)current->data));			// remove process from the list 
				
				copy = g_list_append(copy, temp);								//re-add the process at the end of the list	
			}	*/
		}
	}
							
	totalWaitTime /= size; 	//calculate average wait time of algorithm
	
	g_list_free(copy); 		// deallocate memory of the copy list

	printf("%f\n\n", totalWaitTime);	
}

void RoundRobin(GList* list, int quantum){
	
	//for traversing the list
	GList* current;
	
	GList* copy = g_list_copy_deep(list, (GCopyFunc)copyList, NULL);
	
	//temporarily store a Process
	Process* temp;
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(copy);
	
	//found used to keep the processor idle until a process arrives
	int found, i;

	copy = g_list_sort(copy, (GCompareFunc)compareArrival);
							
	while(copy){
					
		//restart found, at the beginning we don´t know if there are any proceses waiting
		found = 0;
			
		//traverse the list to find the next process
		for(current = copy; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed ){ 			//use the first process of the list that has arrived
																														
				found = 1;														//if process is found stop searching
				break;
			}	
		}
		
		if(found){
						
			for(i = 0; i < quantum; i++){
				
				timePassed ++; 													// increase time passed by the quantum
				((Process*)current->data)->burstLeft -= 1; 						//reduce the burst by the quantum
				
				if(((Process*)current->data)->burstLeft == 0)
					break;														// make sure that the process stops when it reaches 0
			}
			
			if(((Process*)current->data)->burstLeft == 0){
				
				int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->firstArriveTime; //get the wait time to the process
				
				totalWaitTime += waitTime;										//add the process waitTime to the total waitTime
				
				copy = g_list_remove(copy, ((Process*)current->data));			// remove process from the list 
									
			}	
			else{  	//send the process to the end of the list
				
				temp = ((Process*)current->data);										//store a process temporarily
				
				temp->arriveTime = timePassed;											// Update the arriveTime of the process
					
				copy = g_list_remove(copy, ((Process*)current->data));					// remove process from the list 
					
				copy = g_list_insert_sorted(copy, temp, (GCompareFunc)compareArrival); 	//re-add the process at its new position
			}
		}
		
		else
			timePassed++;														//if there´s no process in the queue, increase time by 1
	}
		
	
	totalWaitTime /= size;		//calculate average wait time of algorithm
	
	g_list_free(copy); 			// deallocate memory of the copy list

	printf("%f\n\n", totalWaitTime);	
}

void FirstCome(GList* list){
		
	GList* copy = g_list_copy_deep(list, (GCopyFunc)copyList, NULL);
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0;
	
	//get the size of the process list
	int size = g_list_length(copy);
			
	//sort the list one time only
	copy = g_list_sort(copy, (GCompareFunc)compareArrival);
	
	
	while(copy){
											
		if(((Process*)copy->data)->arriveTime > timePassed){ 			//only use the first process of the list that HAS arrived
																												
			timePassed++; 												// if a process has not arrived, increase the time and recheck
		}
		
		else{
			
			timePassed += ((Process*)copy->data)->burst;  				// increase timePassed by burst amount
						
			int waitTime = timePassed - ((Process*)copy->data)->burst - ((Process*)copy->data)->arriveTime; //get the wait time to the process
				
			totalWaitTime += waitTime;									//add the process waitTime to the total waitTime
				
			copy = g_list_remove(copy, ((Process*)copy->data));			// remove process from the list 
		}
	}
		
	
	totalWaitTime /= size;	//calculate average wait time of algorithm
	
	g_list_free(copy); 		// deallocate memory of the copy list

	printf("%f\n\n", totalWaitTime);	
}


//This function is used for the priority and SJF  NonPreemptive algorithms
//It needs the processList header and a string to diferentiate betweeen the diferent algorithms
void NonPreemptive(GList* list, char *criteria){
	
	//for traversing the list
	GList* current;
	
	GList* copy = g_list_copy_deep(list, (GCopyFunc)copyList, NULL);
	
	// used to keep track of the waitTime of all the processes
	float totalWaitTime = 0;
	
	//measures the total time passed during the whole function
	int timePassed = 0 , found;
	
	//get the size of the process list
	int size = g_list_length(copy);
					
	//Sort the list depending on the chosen criteria
	if(!strcmp(criteria, "burst"))
		copy = g_list_sort(copy, (GCompareFunc)compareBurst);
	
	else
		copy = g_list_sort(copy, (GCompareFunc)comparePriority);
		
	
	//while there are processes to process						
	while(copy){
		
		found = 0;
		
		//traverse the list to find the next process
		for(current = copy; current; current = current->next){
			
			if(((Process*)current->data)->arriveTime <= timePassed){		//use the first process of the list that has arrived
																													 
				found = 1;													//if process is found stop searching
				break;
			}	
		}
		
		if(found){
			
			timePassed += ((Process*)current->data)->burst;  				// increase timePassed by burst amount
						
			int waitTime = timePassed - ((Process*)current->data)->burst - ((Process*)current->data)->arriveTime; //get the wait time to the process
				
			totalWaitTime += waitTime;										//add the process waitTime to the total waitTime
							
			copy = g_list_remove(copy, ((Process*)current->data));			// remove process from the list 
		}
		
		else
			timePassed++; 													// if a process has not arrived, increase the time and recheck
	}
		
	
	totalWaitTime /= size;		//calculate average wait time of algorithm
	
	g_list_free(copy); 			// deallocate memory of the copy list

	printf("%f\n\n", totalWaitTime);
}
