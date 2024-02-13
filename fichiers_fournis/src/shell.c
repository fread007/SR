/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

void clear_pid(){
	while(waitpid(-1,NULL,0)!=-1){}
}

int main()
{
	while (1) {
		struct cmdline *l;
		int i, j;

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

		if (l->in != NULL) {
			printf("in: %s\n", l->in);
			int doc_in = open(l->in,O_RDONLY);
			dup2(doc_in,0);
		}
		if (l->out != NULL) {
			printf("out: %s\n", l->out);
			int doc_out = open(l->out,O_WRONLY ,O_CREAT);
			dup2(doc_out,1);
		}


		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
			if(!strcmp(l->seq[i][0],"quit")){
				printf("quit\n");
				clear_pid();
				exit(0);
			}
			
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}

			if(Fork()==0){
				execvp(cmd[0],cmd);
				exit(0);
			}
			clear_pid();

			printf("\n");
		}
	}
}
