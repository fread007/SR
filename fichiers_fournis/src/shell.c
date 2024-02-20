/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

#define FIFO_NAME "./fifo/fifo%d"

void debbug(int val_retour, char* nom){
	switch (val_retour) {
		case 1:
			fprintf(stderr,"%s: command not found.\n", nom);
			break;
		case 2:
			switch (errno) {
				case EACCES:
					fprintf(stderr,"%s: Permission denied.\n", nom);
					break;
				case ENOENT:
					fprintf(stderr,"%s: File not found.\n", nom);
					break;
				default : 
					fprintf(stderr,"%s: Error number %d.\n", nom, errno);
					break;
			}
			break;
		case 3:
			fprintf(stderr,"%s: Closing file error number %d.\n", nom, errno);
			break;
		case 4:
			switch (errno) {
				case EACCES :
					fprintf(stderr, "%s: A repertory in the path doesn't allow the execution.\n", nom);
				case ENOENT:
					fprintf(stderr,"%s: A repertory of the path doesn't exist or links to NULL.\n", nom);
					break;
				case EROFS:
					fprintf(stderr,"%s: The pathname is on a read-only file system.\n", nom);
				default : 
					fprintf(stderr,"%s: Error number %d.\n", nom, errno);
					break;
			}
			break;
		case 5:
			switch (errno) {
				case EBADF:
					fprintf(stderr, "%s: oldfd is not an open file descriptor, or newfd is not a valid file descriptor.\n", nom);
					break;
				case EBUSY:
					fprintf(stderr, "%s: (Linux only) This value may be returned by dup2() in a race condition with open(2) and dup().\n", nom);
					break;
				case EINTR:
					fprintf(stderr, "%s: call interrupted by a signal\n", nom);
					break;
				case EMFILE:
					fprintf(stderr, "%s: The process already has the maximum number of file descriptors open concurrently and is trying to open one more.\n", nom);
					break;
			}
			break;
	}	
}

void clear_pid(){
	while(waitpid(-1,NULL,0)!=-1){}
}


void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout){
	if(l->in != NULL){
		int e = close(doc_in);
		if (e == -1) {
			debbug(3, l->in);
		}		
		l->in = NULL;
		if (dup2(new_stdin,0) == -1){
			debbug(4, "dup2");
		};
	}
	if(l->out != NULL){
		int e = close(doc_out);
		if (e == -1) {
			debbug(3, l->in);
		}
		l->out = NULL;
		if (dup2(new_stdout,1) == -1) {
			debbug(4, "dup2");
		};
	}
}

void exec_quit(){
	printf("quit\n");
	clear_pid();
	exit(0);
}


int main()
{
	while (1) {
		struct cmdline *l;
		int i, j;
		int doc_in,doc_out;
		int fifo_fd, fifo_fd1;
		int new_stdin,new_stdout;
		char fifo_actuel_name[50];
		

		printf("kintama> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}


		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];

			printf("seq[%d]: ", i);

			if(!strcmp(l->seq[i][0],"quit")){
				exec_quit();
			}
			if (l->out != NULL && l->seq[i+1]==0) {
				printf("out: %s\n", l->out);
				doc_out = open(l->out,O_WRONLY | O_CREAT | O_TRUNC,0752);
				if (doc_out == -1) {
					debbug(2, l->in);
				}
				new_stdout = dup(0);
				if (dup2(doc_out,1) == -1){
					debbug(4, "dup2");
				};
			}

			if (l->in != NULL && l->seq[i+1]==0) {
				printf("in: %s\n", l->in);
				doc_in = open(l->in,O_RDONLY,0);
				if (doc_in == -1) {
					debbug(2, l->in);
				}
				new_stdin = dup(0);
				if (dup2(doc_in,0) == -1){
					debbug(4, "dup2");
				};

			}

			if(Fork()==0){
				if (i!=0 && l->seq[i+1] != 0){
					sprintf(fifo_actuel_name, FIFO_NAME, i-1);
					if(mkfifo(fifo_actuel_name,0777) == -1){
						if (errno != EEXIST){
							debugg(4, "./fifo");
						}
					};
					fifo_fd=open(fifo_actuel_name,O_RDONLY);
					if (fifo_fd == -1) {
						debbug(2, fifo_actuel_name);
					}

					sprintf(fifo_actuel_name, FIFO_NAME, i);
					if(mkfifo(fifo_actuel_name,0777) == -1){
						if (errno != EEXIST){
							debugg(4, "./fifo");
						}
					};
					fifo_fd1=open(fifo_actuel_name,O_WRONLY);
					if (fifo_fd1 == -1) {
						debbug(2, fifo_actuel_name);
					}

					if (dup2(fifo_fd,0) == -1){
						debbug(4, "dup2");
					};
					if (dup2(fifo_fd1,1) == -1){
						debbug(4, "dup2");
					};
					
				}
				else if (i!=0) {
					sprintf(fifo_actuel_name, FIFO_NAME, i-1);
					if(mkfifo(fifo_actuel_name,0777) == -1){
						if (errno != EEXIST){
							debugg(4, "./fifo");
						}
					};
					fifo_fd=open(fifo_actuel_name,O_RDONLY);
					if (dup2(fifo_fd,0) == -1){
						debbug(4, "dup2");
					};
				}
				else if (l->seq[i+1] != 0){
					sprintf(fifo_actuel_name, FIFO_NAME, i);
					if(mkfifo(fifo_actuel_name,0777) == -1){
						if (errno != EEXIST){
							debugg(4, "./fifo");
						}
					};
					fifo_fd=open(fifo_actuel_name,O_WRONLY); 
					if (dup2(fifo_fd,1) == -1) {
						debbug(4, "dup2");
					};
				}
				int e=execvp(cmd[0],cmd);
				if (e == -1) {
					debbug(1, cmd[0]);
				}
				exit(0);
				
			}

			if(l->seq[i+1]==0){
				clear_redirection(l,doc_in,doc_out,new_stdin,new_stdout);
			}
			
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}

			printf("\n");
		}
		clear_pid();
	}
}