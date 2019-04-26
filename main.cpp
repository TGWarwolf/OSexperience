// OSshiyan.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define READY 0
#define BLOCK 1
#define RUNNING 2
struct process {
	char PID[10];
	/*int res_required[4]; //total rescources this process requires*/
	int res_occupied[4]; //rescource that have be allocation 
	int status;   //0:ready  1:block 2:running
	process * parent;
	process * child;
	process * child_next; //to solve the condition of more than one children
	int priority;
};

struct queue {
	process * Pro;
	queue * next;
};
struct resource {
	char RID[10];
	int total_amount;
	int available_amount;
	queue * waiting_list;
};

void Init(queue ** ready_Q, resource ** Res, queue * running_pro);
void Create(queue ** ready_Q, char * name, int status, int priority, queue * running_pro);
void Scheduler(queue ** ready_Q, queue * running_pro);
void Delete();
void Request();
void Release();
void TimeOut(queue ** ready_Q,queue * running_pro);
void List(queue ** ready_Q, resource ** Res, queue * running_pro,int status);
void ListReady(queue * Q_pointer, int priority);
char * GetName(char *cmd);
int GetNumber(char * cmd);
queue * GetRunPro(queue ** ready_Q);

int main(){
	char cmd[20];
	int cmd_i = 0;
	queue * running_pro = (queue *)malloc(sizeof(queue));
	running_pro->Pro = NULL;
	queue * ready_queue[3];
	resource * res[4];
	while (1) {
		printf("shell>");
		cmd_i = 0;
		while ((cmd[cmd_i] = getchar()) != '\n'){
			cmd_i++;
		}
		cmd[cmd_i] = '\0';
		if (0==strcmp(cmd, "-init")) {
			Init(ready_queue,res,running_pro);
		}
		else if (0==strncmp(cmd, "-cr ", 4)) {
			Create(ready_queue,GetName(cmd),READY,GetNumber(cmd),running_pro);
		}/*
		else if (strncmp(cmd, "-de ", 4)) {
			Delete();
		}
		else if (strncmp(cmd, "-req ", 5)) {
			Request();
		}
		else if (strncmp(cmd, "-rel ", 5)) {
			Release();
		}*/
		else if (0==strcmp(cmd, "-to")) {
			TimeOut(ready_queue, running_pro);
		}
		else if (0==strcmp(cmd, "-list ready")) {
			List(ready_queue, res, running_pro,READY);
		}
		/*
		else if (strcmp(cmd, "-list block")) {
			List("block");
		}
		else if (strcmp(cmd, "-list res")) {
			List("resourse");
		}
		*/
		else {
			printf("Please enter the right command\n");
		}
		fflush(stdin);
	}
    return 0;
}

void Init(queue ** ready_Q, resource ** Res, queue * running_pro) {

	for (int i = 0; i < 4; i++) {
		//Initial the Rescourses ID,amount,list
		Res[i] = (resource *)malloc(sizeof(resource));
		Res[i]->RID[0] = 'R';
		Res[i]->RID[1] = '1' + i;
		Res[i]->RID[2] = '\0';
		Res[i]->total_amount = i+1;
		Res[i]->available_amount = i+1;
		Res[i]->waiting_list = NULL;
	}
	for (int i = 0; i < 3; i++) {
		//Initial the ready queue
		ready_Q[i] = (queue *)malloc(sizeof(queue));
		ready_Q[i]->next = NULL;
		ready_Q[i]->Pro = NULL;//head
	}

	//Initial the init_pro
	process * init_pro = (process *)malloc(sizeof(process));
	strcpy(init_pro->PID, "init_pro");
	init_pro->priority = 0;
	init_pro->status = READY;//ready
	for (int i = 0; i < 4; i++) {
		init_pro->res_occupied[i] = 0;
	}
	init_pro->parent = NULL;
	init_pro->child = NULL;
	init_pro->child_next = NULL;

	//Append
	queue * init_Q = (queue *)malloc(sizeof(queue));
	init_Q->Pro = init_pro;
	init_Q->next = NULL;
	ready_Q[0]->next = init_Q;

	Scheduler(ready_Q,running_pro);
}
void Create(queue ** ready_Q, char * name, int status, int priority, queue * running_pro) {
	queue * temp = (queue *)malloc(sizeof(queue));
	temp->next = ready_Q[priority]->next;//head insert
	temp->Pro = (process *)malloc(sizeof(process));
	if (!ready_Q[priority]->next) {
		ready_Q[priority]->next = (queue *)malloc(sizeof(process));
		ready_Q[priority]->next = temp;
	}
	else {
		temp->next = ready_Q[priority]->next;
		ready_Q[priority]->next = temp;
	}
	strcpy(temp->Pro->PID, name);
	temp->Pro->priority = priority;
	temp->Pro->status = READY;//ready
	for (int i = 0; i < 4; i++) {
		temp->Pro->res_occupied[i] = 0;
	}
	temp->Pro->parent = running_pro->Pro;
	if (!running_pro->Pro->child) {
		//running_pro->Pro = (process *)malloc(sizeof(process));
		running_pro->Pro->child = temp->Pro;
	}
	else {
		process * temp_child = running_pro->Pro->child_next;
		if (!temp_child) {
			temp_child = (process *)malloc(sizeof(process));
		}
		running_pro->Pro->child_next = temp->Pro;
		temp->Pro->child_next = temp_child;
	}
	temp->Pro->child = NULL;
	Scheduler(ready_Q, running_pro);
}

void Scheduler(queue ** ready_Q, queue * running_pro){
	queue * temp_running = (queue *)malloc(sizeof(queue));
	if (NULL!=ready_Q[2]->next) {
		temp_running = ready_Q[2];
	}
	else if(NULL!=ready_Q[1]->next){
		temp_running = ready_Q[1];
	}
	else {
		temp_running = ready_Q[0];
	}
	while (temp_running->next->next) {
		temp_running = temp_running->next;
	}
	//When call Init or TimeOut;
	if (!running_pro->Pro) {
		temp_running->next->Pro->status = RUNNING;
		running_pro->Pro = temp_running->next->Pro;
		running_pro->next = NULL;
		temp_running->next = NULL;
		printf("Process %s is running!\n", running_pro->Pro->PID);
		return;
	}
	int pri = running_pro->Pro->priority;
	if (temp_running->next->Pro->priority <= pri) {
		printf("Process %s is running!\n", running_pro->Pro->PID);
		return;
	}
	running_pro->Pro->status = READY;
	temp_running->next->Pro->status = RUNNING;
	printf("Process %s is ready,", running_pro->Pro->PID);
	queue * temp_ready = (queue *)malloc(sizeof(queue));
	temp_ready->Pro = running_pro->Pro;
	temp_ready->next = ready_Q[pri]->next;
	ready_Q[pri]->next = temp_ready;
	running_pro->Pro = temp_running->next->Pro;
	temp_running->next = NULL;
	printf("Process %s is running!\n", running_pro->Pro->PID);
}
void TimeOut(queue ** ready_Q, queue * running_pro) {
	int pri = running_pro->Pro->priority;
	queue * temp_next = (queue *)malloc(sizeof(queue));
	running_pro->Pro->status = READY;
	temp_next->Pro = ready_Q[pri]->next->Pro;
	temp_next->next = ready_Q[pri]->next->next;
	if (!ready_Q[pri]->next) {
		ready_Q[pri]->next = (queue *)malloc(sizeof(queue));
	}
	//put the running_pro in the queue;
	ready_Q[pri]->next->Pro = running_pro->Pro;
	ready_Q[pri]->next->next = temp_next;
	printf("Process %s is ready,", running_pro->Pro->PID);
	running_pro->next = NULL;
	running_pro->Pro = NULL;
	//now pre-running_pro is in the ready queue;
	Scheduler(ready_Q, running_pro);
}
void List(queue ** ready_Q, resource ** Res, queue * running_pro, int status) {
	switch (status) {
	case RUNNING:
		printf("Running:%s\n", running_pro->Pro->PID);
		return;
	case READY:
		for (int i = 2; i >= 0; i--) {
			ListReady(ready_Q[i], i);
		}
		return;
	case BLOCK:
		return;
	default:
		return;
	}
}
void ListReady(queue * Q_pointer, int priority) {
	if (NULL== Q_pointer->next) {
		printf("Ready queue of priority %d:", priority);
		if (NULL != Q_pointer->Pro) {
			printf("%s", Q_pointer->Pro->PID);
		}
		else {
			printf("\n");
		}
	}
	else {
		ListReady(Q_pointer->next, priority);
		if (NULL != Q_pointer->Pro->PID) {
			printf("-%s", Q_pointer->Pro->PID);
		}
		else {
			printf("\n");
		}
		
	}
}
char * GetName(char *cmd) {
	char *name = (char *)malloc(10 * sizeof(char));
	int i = 0, j = 0;
	while (' '!= cmd[i]) {
		i++;
	}
	while (' ' == cmd[i]) {
		i++;
	}
	while ((' ' != cmd[i])&&(j<9)&&(i<20)) {
		name[j] = cmd[i];
		j++;
		i++;
	}
	name[j] = '\0';
	return name;
}
int GetNumber(char * cmd) {
	int flag = 0;
	for (int i = 0; i < 20; i++) {
		if ( ' ' == cmd[i] ) {
			if (0 == flag) {
				flag++;
			}
			if (2 == flag) {
				flag++;
			}
		}
		if (' ' != cmd[i]) {
			if (1 == flag) {
				flag++;
			}
			if (3 == flag) {
				return cmd[i] - '0';
			}
			
		}
		
	}

	return 0;
}
queue * GetRunPro(queue ** ready_Q) {
	return NULL;
}