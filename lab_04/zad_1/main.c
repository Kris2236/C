#include<stdio.h>
#include<signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

int handler_call = 0;

void is_pending(int signum) {
    sigset_t test_set;
    sigpending(&test_set);
    if(sigismember(&test_set, signum) != 0)
        printf("\tInside handler signal %d is pending\n", signum);
}

void sig_handler(int signum){
    handler_call++;
    if(handler_call > 1)
        return;

    printf("\tInside handler function\n");
    sleep(1);

    kill(getpid(), signum);
    is_pending(signum);
}

int parse(char *command, int sig_nr) {
    handler_call = 0;

    if(sig_nr == 9 || sig_nr == 19) {
        signal(sig_nr, SIG_DFL);
        kill(getpid(), sig_nr);
        return 0;
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    act.sa_flags = SA_NODEFER;

    if(strcmp(command, "ignore") == 0) {
        printf("Signal %d will be %s!\n", sig_nr, command);
        act.sa_handler = SIG_IGN;
    } else if(strcmp(command, "handle") == 0) {
        printf("Signal %d will be %s!\n", sig_nr, command);
        act.sa_handler = sig_handler;
    } else if(strcmp(command, "mask") == 0) {
        printf("Signal %d will be %s!\n", sig_nr, command);
        act.sa_handler = sig_handler;
        sigaddset(&act.sa_mask, sig_nr);
    } else {
        printf("Wrong command!\n");
        return 0;
    }

    sigaction(sig_nr, &act, NULL);      // add new signal action

    int send_sig = raise(sig_nr);   //kill(getpid(), sig_nr);
    if(send_sig != 0) {
        printf("SEND SIGNAL: %d\n", send_sig);
        printf(("Signal sending failed!\n"));
    }

    handler_call = 0;

    int child_pid = fork();
    if(child_pid == -1)
    {
        perror("Error occurred with process!\n");
        exit(2);
    }
    else if(child_pid == 0)         // child process
    {
        printf("\nChild process do:\n");
        raise(sig_nr);
        exit(0);
    }
    else            // main process is waiting for child
    {
        int res = wait(NULL);
        if(res == -1)
            printf("Ther is no process to wait for.\n");
        else
            printf("%d proces was finished.\n", res);
        printf("---------------\n\n");
    }

    return 1;
}


int main(int argc, char *argv[]){

    if(argc == 4 && atoi(argv[1]) == getpid()) {
        printf("EXEC do: %s %s\n", argv[2], argv[3]);
        parse(argv[2], atoi(argv[3]));
        exit(0);
    } else {
        while (1){
            int sig_nr;
            char signal[128];

            printf("Type:\t[operation {ignore, handle, mask}] [signal nr] recommended 10 (0 to exit)\n");
            scanf("%s %d", signal, &sig_nr);

            if(sig_nr == 0 || signal[0] == '0')
                break;

            if(parse(signal, sig_nr)) {
                char pid_str[12];
                sprintf(pid_str, "%d", getpid());
                char sig_nr_str[3];
                sprintf(sig_nr_str, "%d", sig_nr);

                char * args[] = {"./zad_1", pid_str, signal, sig_nr_str, NULL};
                execvp("./zad_1", args);                    // EXEC
            }
        }
    }

    return 0;
}






/*
 * Ustawl sygnały oraz maski
 *
 * Wykonaj testując dziedziczenie sygnałów za pomocą SIGUSR1
 *  - fork
 *  - exec
 *
 *  Wyślij do samego siebie sygnał
 *
 *   ignore, handler, mask lub pending
 * */
