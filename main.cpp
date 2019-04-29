// OSshiyan.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define READY 0
#define BLOCK 1
#define RUNNING 2
#define RESOURCE 3
#define NOWAITING 999

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
void QueueIn(queue ** ready_Q,int priority, queue * target);
void QueueInRes(resource ** res, int index, queue * target);
void Scheduler(queue ** ready_Q, queue * running_pro);
void Delete(queue ** ready_Q, char * name, queue * running_pro, resource ** res);
void Kill(queue * target, resource ** res);
void Request(queue ** ready_Q, char * res_name, int amount, queue * running_pro, resource ** res);
void Release(queue ** ready_Q, char * res_name, int amount, queue * running_pro, resource ** res);
void ReleaseResource(resource ** res, int amount, queue * running_pro,int index);
int Unblock(queue ** ready_Q, resource ** res,int index);
void TimeOut(queue ** ready_Q,queue * running_pro);
void List(queue ** ready_Q, resource ** res, queue * running_pro,int status);
void ListReady(queue * Q_pointer, int priority);
void ListBlock(queue * Q_pointer, int index);
char * GetName(char *cmd);
int GetNumber(char * cmd);
queue * FindProPre(queue ** ready_Q, resource ** res, queue * running_pro, char * name);

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
		}
		else if (0==strncmp(cmd, "-de ", 4)) {
			Delete(ready_queue, GetName(cmd), running_pro, res);
		}
		else if (0==strncmp(cmd, "-req ", 5)) {
			Request(ready_queue, GetName(cmd), GetNumber(cmd), running_pro, res);
		}
		else if (0==strncmp(cmd, "-rel ", 5)) {
			Release(ready_queue, GetName(cmd), GetNumber(cmd), running_pro, res);
		}
		else if (0==strcmp(cmd, "-to")) {
			TimeOut(ready_queue, running_pro);
		}
		else if (0==strcmp(cmd, "-list ready")) {
			List(ready_queue, res, running_pro,READY);
		}
		
		else if (0==strcmp(cmd, "-list block")) {
			List(ready_queue, res, running_pro, BLOCK);
		}
		else if (0==strcmp(cmd, "-list res")) {
			List(ready_queue, res, running_pro, RESOURCE);
		}
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
		//Create the head of the waiting queue
		Res[i]->waiting_list = (queue *)malloc(sizeof(queue));
		Res[i]->waiting_list->next = NULL;
		Res[i]->waiting_list->Pro = NULL;
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

	QueueIn(ready_Q, priority, temp);
	/*
	if (!ready_Q[priority]->next) {
		ready_Q[priority]->next = (queue *)malloc(sizeof(process));
		ready_Q[priority]->next = temp;
	}
	else {
		temp->next = ready_Q[priority]->next;
		ready_Q[priority]->next = temp;
	}*/
	
	Scheduler(ready_Q, running_pro);
}
void Delete(queue ** ready_Q, char * name, queue * running_pro, resource ** res) {
	if (0 == running_pro->Pro->priority) {
		printf("Can't delete init_pro!\n");
		return;
	}
	if (0 == strcmp(running_pro->Pro->PID, name)) {
		Kill(running_pro,res);
	}
}
void Kill(queue * target, resource ** res) {
	//GG 只能找名字了
	if (target->Pro->child) {
		;
	}
}
void Request(queue ** ready_Q, char * res_name, int amount, queue * running_pro, resource ** res) {
	int res_index = res_name[1] - '1';
	if (res_index > 3 || res_index < 0 || ('r' != res_name[0] && 'R' != res_name[0]) || amount <= 0) {
		printf("Please enter the right rescorce.\n");
		return;
	}
	//Error amount;
	if (amount+running_pro->Pro->res_occupied[res_index] > res[res_index]->total_amount) {
		printf("There aren't %d Resource %s in total.\n", amount, res[res_index]->RID);
		return;
	}
	//Init_pro can't request resource;
	if (0 == running_pro->Pro->priority) {
		printf("Init_pro can't request resource.\n");
		return;
	}
	//There is enough resource;
	if (amount <= res[res_index]->available_amount) {
		printf("Process %s requested %d Resouce %s.\n",running_pro->Pro->PID, amount, res[res_index]->RID);
		running_pro->Pro->res_occupied[res_index] += amount;
		res[res_index]->available_amount -= amount;
	}
	else {
		//There is not enough resource;
		queue * temp_next = (queue *)malloc(sizeof(queue));
		running_pro->Pro->status = BLOCK;
		running_pro->Pro->res_occupied[res_index] += 10 * amount;//decade is the resource not enough
		temp_next->next = running_pro->next;
		temp_next->Pro = running_pro->Pro;
		QueueInRes(res, res_index, temp_next);

		printf("Process %s is blocked,", running_pro->Pro->PID);

		running_pro->next = NULL;
		running_pro->Pro = NULL;
		Scheduler(ready_Q, running_pro);
	}

}
void Release(queue ** ready_Q, char * res_name, int amount, queue * running_pro, resource ** res) {
	int res_index = res_name[1] - '1';
	if (res_index > 3 || res_index < 0 || ('r' != res_name[0] && 'R' != res_name[0]) || amount <= 0) {
		printf("Please enter the right rescorce.\n");
		return;
	}
	//Error amount;
	if (amount > running_pro->Pro->res_occupied[res_index] % 10) {
		printf("There aren't %d Resource %s the process %s occupied.\n", amount, res[res_index]->RID, running_pro->Pro->PID);
		return;
	}
	//Init_pro can't request resource;
	if (0 == running_pro->Pro->priority) {
		printf("Init_pro can't request resource.\n");
		return;
	}
	ReleaseResource(res, amount, running_pro, res_index);
	while(Unblock(ready_Q,res,res_index)<res[res_index]->available_amount) {
		;
	}
	Scheduler(ready_Q, running_pro);
}
void ReleaseResource(resource ** res, int amount, queue * running_pro,int index) {
	res[index]->available_amount += amount;
	running_pro->Pro->res_occupied[index] -= amount;
}
int Unblock(queue ** ready_Q, resource ** res,int index) {
	if (!res[index]->waiting_list->next) {
		return NOWAITING;
	}
	queue * temp_ready = (queue *)malloc(sizeof(queue));
	temp_ready = res[index]->waiting_list;
	while (temp_ready->next->next) {
		temp_ready = temp_ready->next;
	}
	int request_amount = temp_ready->next->Pro->res_occupied[index] / 10;
	if (request_amount > res[index]->available_amount) {
		return request_amount;
	}
	int pri = temp_ready->next->Pro->priority;
	res[index]->available_amount -= request_amount;
	temp_ready->next->Pro->res_occupied[index] -= (request_amount * 10 - request_amount);
	temp_ready->next->Pro->status = READY;
	QueueIn(ready_Q, pri, temp_ready->next);
	printf("Wake up Process %s,", temp_ready->next->Pro->PID);
	temp_ready->next = NULL;
	if (!temp_ready->Pro) {
		return NOWAITING;
	}
	return temp_ready->Pro->res_occupied[index] / 10;

}
void QueueIn(queue ** ready_Q,int priority, queue * target) {
	target->next = ready_Q[priority]->next;
	if (!ready_Q[priority]->next) {
		ready_Q[priority]->next = (queue *)malloc(sizeof(process));
	}
	ready_Q[priority]->next = target;
	
}
void QueueInRes(resource ** res, int index, queue * target) {
	target->next = res[index]->waiting_list->next;
	if (!res[index]->waiting_list->next) {
		res[index]->waiting_list->next = (queue *)malloc(sizeof(process));
	}
	res[index]->waiting_list->next = target;
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
	//Find the next ready process's pre;
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
	//ready_Q[pri]->next = temp_ready;
	QueueIn(ready_Q, pri, temp_ready);
	running_pro->Pro = temp_running->next->Pro;
	temp_running->next = NULL;
	printf("Process %s is running!\n", running_pro->Pro->PID);
}
void TimeOut(queue ** ready_Q, queue * running_pro) {
	int pri = running_pro->Pro->priority;
	queue * temp_next = (queue *)malloc(sizeof(queue));
	running_pro->Pro->status = READY;
	temp_next->next = running_pro->next;
	temp_next->Pro = running_pro->Pro;
	QueueIn(ready_Q, pri, temp_next);

	printf("Process %s is ready,", running_pro->Pro->PID);
	
	running_pro->next = NULL;
	running_pro->Pro = NULL;
	//now pre-running_pro is in the ready queue;
	Scheduler(ready_Q, running_pro);
}
void List(queue ** ready_Q, resource ** res, queue * running_pro, int status) {
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
		for (int i = 0; i < 4; i++) {
			ListBlock(res[i]->waiting_list, i);
		}
		return;
	case RESOURCE:
		for (int i = 0; i < 4; i++) {
			printf("Resource %s:%d\n", res[i]->RID, res[i]->available_amount);
		}
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
void ListBlock(queue * Q_pointer, int index) {
	if (NULL == Q_pointer->next) {
		printf("Block queue of R%d:", index + 1);
		if (NULL != Q_pointer->Pro) {
			printf("%s", Q_pointer->Pro->PID);
		}
		else {
			printf("\n");
		}
	}
	else {
		ListBlock(Q_pointer->next, index);
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
queue * FindProPre(queue ** ready_Q, resource ** res, queue * running_pro, char * name) {
	//Not debug yet;
	if (0 == strcmp(running_pro->Pro->PID, name)) {
		return running_pro;
	}
	queue * temp = (queue *)malloc(sizeof(queue));
	for (int index=2; index > 0; index--) {
		if (NULL != ready_Q[index]->next) {
			temp = ready_Q[index];
			while (temp->next->next) {
				if (0 == strcmp(temp->next->Pro->PID, name)) {
					return temp;
				}
				temp = temp->next;
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		if (NULL != res[i]->waiting_list->next) {
			temp = res[i]->waiting_list;
			while (temp->next->next) {
				if (0 == strcmp(temp->next->Pro->PID, name)) {
					return temp;
				}
				temp = temp->next;
			}
		}
	}
	return NULL;
	
}