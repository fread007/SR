#ifndef __EXEC_CMD__
#define __EXEC_CMD__

void debbug(int val_retour, char* nom);

void handler(int sig);

void clear_pid();

void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout);

void exec_quit();

void execut_commande (struct cmdline *l,char* fifo_actuel_name, int fifo_fd,int fifo_fd1, char **cmd,int i);
#endif