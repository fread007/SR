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
	}	
}

void handler(int sig){
	pid_t pid;
	while((pid=waitpid(-1,NULL,WNOHANG))>0){}
}

void clear_pid(){
	while(waitpid(0,NULL,0)!=-1){}
}


void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout){
	if(l->in != NULL){
		int e = close(doc_in);
		if (e == -1) {
			debbug(3, l->in);
		}		
		l->in = NULL;
		dup2(new_stdin,0);
	}
	if(l->out != NULL){
		int e = close(doc_out);
		if (e == -1) {
			debbug(3, l->in);
		}
		l->out = NULL;
		dup2(new_stdout,1);
	}
}

void exec_quit(){
	printf("quit\n");
	clear_pid();
	exit(0);
}

void execut_commande (struct cmdline *l,char* fifo_actuel_name, int fifo_fd,int fifo_fd1, char **cmd,int i){
    if(Fork()==0){
        if (i!=0 && l->seq[i+1] != 0){
            sprintf(fifo_actuel_name, FIFO_NAME, i-1);
            mkfifo(fifo_actuel_name,0777);
            fifo_fd=open(fifo_actuel_name,O_RDONLY);

            sprintf(fifo_actuel_name, FIFO_NAME, i);
            mkfifo(fifo_actuel_name,0777);
            fifo_fd1=open(fifo_actuel_name,O_WRONLY);
            

            dup2(fifo_fd,0);
            dup2(fifo_fd1,1);
            
        }
        else if (i!=0) {
            sprintf(fifo_actuel_name, FIFO_NAME, i-1);
            mkfifo(fifo_actuel_name,0777);
            fifo_fd=open(fifo_actuel_name,O_RDONLY);
            dup2(fifo_fd,0);
        }
        else if (l->seq[i+1] != 0){
            sprintf(fifo_actuel_name, FIFO_NAME, i);
            mkfifo(fifo_actuel_name,0777);
            fifo_fd=open(fifo_actuel_name,O_WRONLY); 
            dup2(fifo_fd,1);
        }
        int e=execvp(cmd[0],cmd);
        if (e == -1) {
            debbug(1, cmd[0]);
        }
        exit(1);
        
    }
}