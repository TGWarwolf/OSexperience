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
void TimeOut();
void List(char * type);
void PrintPCB();
queue * GetRunPro(queue ** ready_Q);

int main(){
	char cmd[20];
	queue * running_pro = (queue *)malloc(sizeof(queue));
	running_pro->Pro = NULL;
	queue * ready_queue[3];
	resource * res[4];
	while (1) {
		printf("shell>");
		scanf("%s", cmd);
		if (0==strncmp(cmd, "-init",5)) {
			Init(ready_queue,res,running_pro);
		}/*
		else if (strncmp(cmd, "-cr ", 4)) {
			Create(ready_queue,cmd,1,1);
		}
		else if (strncmp(cmd, "-de ", 4)) {
			Delete();
		}
		else if (strncmp(cmd, "-req ", 5)) {
			Request();
		}
		else if (strncmp(cmd, "-rel ", 5)) {
			Release();
		}
		else if (strcmp(cmd, "-to")) {
			TimeOut();
		}
		else if (strcmp(cmd, "-list ready")) {
			List("ready");
		}
		else if (strcmp(cmd, "-list block")) {
			List("block");
		}
		else if (strcmp(cmd, "-list res")) {
			List("resourse");
		}
		else if (strncmp(cmd, "-pr ", 4)) {
			PrintPCB();
		}*/
		else {
			printf("Please enter the right command\n");
		}
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
	if (!ready_Q[priority]->next) {
		ready_Q[priority]->next = (queue *)malloc(sizeof(process));
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
		running_pro->Pro = (process *)malloc(sizeof(process));
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
	//When call Init();
	if (!running_pro->Pro) {
		temp_running->next->Pro->status = RUNNING;
		running_pro->Pro = temp_running->next->Pro;
		running_pro->next = NULL;
		temp_running->next = NULL;
		return;
	}
	int pri = running_pro->Pro->priority;
	if (running_pro->Pro->priority > pri)return;
	running_pro->Pro->status = READY;
	temp_running->next->Pro->status = RUNNING;
	
	queue * temp_ready = (queue *)malloc(sizeof(queue));
	temp_ready->Pro = running_pro->Pro;
	temp_ready->next = ready_Q[pri]->next;
	ready_Q[pri]->next = temp_ready;
	running_pro->Pro = temp_running->next->Pro;
	temp_running->next = NULL;
}

queue * GetRunPro(queue ** ready_Q) {
	return NULL;
}