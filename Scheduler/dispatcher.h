/*
 * Copyright (c) 2017 
 * 
 * File name: Dispatcher.h
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

/************************************************************** 
 *             Declare the functions			              * 
 **************************************************************/

GList* SortProcessList(GList* ProccessList);

void PrintProcessList(GList* processList);

GList* CreateProcess(GList *processList, int pid, int arriveTime, int burst, int priority);

void DestroyList(GList *processList);

void Preemptive(GList* list, char *criteria);

void RoundRobin(GList* list, int quantum);

void FirstCome(GList* list);

void NonPreemptive(GList* list, char *criteria);

