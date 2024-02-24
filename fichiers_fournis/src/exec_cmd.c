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


void handler(int sig){
	pid_t pid;
    //tant qu'il y a un fils zombie ont le termine
	while((pid=waitpid(-1,NULL,WNOHANG))>0){}
}

void clear_pid(){
    //attend tout les processus fils
	while(waitpid(0,NULL,0)!=-1){}
}


void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout){
    //test si il y avais eu une redirection de l'entree
	if(l->in != NULL){
        //ferme le fichier
		int e = close(doc_in);
        //test si il y a eu un probleme
		if (e == -1) {
			debbug(3, l->in);
		}		
        //remer a l'etat initial
		l->in = NULL;
		dup2(new_stdin,0);
	}
    //test si il y avais eu une redirection de sortie
	if(l->out != NULL){
        //ferme le fichier
		int e = close(doc_out);
        //test si il y a eu un probleme
		if (e == -1) {
			debbug(3, l->in);
		}
        //remer a l'etat initial
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
    //cree un fils qui executera une commande du pipe
    if(Fork()==0){
        //si n'est ni le premier ni le dernier
        if (i!=0 && l->seq[i+1] != 0){
            //ont cree un fifo(si il nexiste pas)
            sprintf(fifo_actuel_name, FIFO_NAME, i-1);
            if(mkfifo(fifo_actuel_name,0777) == -1){
                if (errno != EEXIST){
                    debbug(4, "./fifo");
                }
            }
            //on ouvre le fifo en lecture
            fifo_fd=open(fifo_actuel_name,O_RDONLY);
            if (fifo_fd == -1) {
                debbug(2, fifo_actuel_name);
            }

            //ont cree un 2eme fifo(si il nexiste pas)
            sprintf(fifo_actuel_name, FIFO_NAME, i);
            if(mkfifo(fifo_actuel_name,0777) == -1){
                if (errno != EEXIST){
                    debbug(4, "./fifo");
                }
            }
            //on ouvre le 2eme fifo en ecriture
            fifo_fd1=open(fifo_actuel_name,O_WRONLY);
            if (fifo_fd1 == -1) {
                debbug(2, fifo_actuel_name);
            }

            //ont change l'entree standare par le 1er fifo
            if (dup2(fifo_fd,0) == -1){
                debbug(4, "dup2");
            }
            //ont change la sortie standare par le 2eme fifo
            if (dup2(fifo_fd1,1) == -1){
                debbug(4, "dup2");
            }
            
        }
        else if (i!=0) {        //si c'est le dernier
            //ont cree un fifo(si il nexiste pas)
            sprintf(fifo_actuel_name, FIFO_NAME, i-1);
            if(mkfifo(fifo_actuel_name,0777) == -1){
                if (errno != EEXIST){
                    debbug(4, "./fifo");
                }
            }
            //ont l'ouvre en lecture
            fifo_fd=open(fifo_actuel_name,O_RDONLY);
            //ont change l'entree standare
            if (dup2(fifo_fd,0) == -1){
                debbug(4, "dup2");
            }
        }
        else if (l->seq[i+1] != 0){         //si c'est le premier
            //ont cree un fifo(si il nexiste pas)
            sprintf(fifo_actuel_name, FIFO_NAME, i);
            if(mkfifo(fifo_actuel_name,0777) == -1){
                if (errno != EEXIST){
                    debbug(4, "./fifo");
                }
            }
            //ont ouvre le fifo en ecriture
            fifo_fd=open(fifo_actuel_name,O_WRONLY); 
            //ont change la sortie standare
            if (dup2(fifo_fd,1) == -1) {
                debbug(4, "dup2");
            };
        }

        //ont execute la commande
        int e=execvp(cmd[0],cmd);
        //ont teste si il y a eu un probleùe
        if (e == -1) {
            debbug(1, cmd[0]);
        }
        exit(1);
        
    }
}