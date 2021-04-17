#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

enum { MAX_MSG_SIZE = 24 };
char *err_msg;
static int count, number, code_glob, value = 0, error, pid_glob, uid_glob;


void handler( int signo, siginfo_t *info, void * o ) {
    count++;
    number = signo;
    code_glob = info->si_code;
    value = info->si_value.sival_int;
    error = info->si_errno;

    pid_glob = info->si_pid;
    uid_glob = info->si_uid;
}

void handler2(int signum) {
    strcpy(err_msg, "SIGINT encountered.");
    pid_glob = getppid();
    uid_glob = getpid();
}

void signal_queue() {
    printf("1. Normal signal queue - SIGINFO\n");

    int i,pid;
    union sigval sig;
    sigset_t sig_set;

    struct sigaction act;
    sigemptyset( &sig_set );
    sigaddset( &sig_set, SIGUSR1 );

    act.sa_flags = SA_SIGINFO;
    act.sa_mask = sig_set;
    act.sa_handler = (__sighandler_t) &handler;
    sigaction( SIGUSR1, &act, NULL );

    if((pid = fork()) == 0) { // Child process
        pid = getppid();
        for(i=0;i<10;i++) {
            printf("Sending: sig = %d \n", i);
            sig.sival_int = i;  // set signal value
            sigqueue(pid, SIGUSR1, sig);
            sleep(1);
        }

        exit(0);
    }

    // Parent process -----------
    while(1) {
        printf("Signal received: counter = %d, sig nr= %d, code= %d, val= %d, errno= %d, PID= %d, UID= %d\n",
               count,number,code_glob,value, error, pid_glob, uid_glob);
        sleep(1);
        if(value == 9) break;
    }
}

void signal_child() {
    printf("1. signal queue - SIGCHLD, no value information, but error information is available\n");

    int i,pid;
    union sigval sig;
    sigset_t sig_set;

    struct sigaction act;
    sigemptyset( &sig_set );
    sigaddset( &sig_set, SIGUSR1 );

    act.sa_flags = SIGCHLD;
    act.sa_mask = sig_set;
    act.sa_handler = (__sighandler_t) &handler;
    sigaction( SIGUSR1, &act, NULL );

    if((pid = fork()) == 0) { // Child process
        pid = getppid();
        for(i=0;i<10;i++) {
            printf("Sending: sig = %d \n", i);
            sig.sival_int = i;  // set signal value
            sigqueue(pid, SIGUSR1, sig);
            sleep(1);
        }

        exit(0);
    }

    // Parent process -----------
    int end = 15;
    while(1) {
        printf("Signal received: counter = %d, sig nr= %d, code= %d, val= %d, errno= %d, PID= %d, UID= %d\n",
               count, number, code_glob, value, error, pid_glob, uid_glob);
        sleep(1);

        if(end == 0) break;
        end--;
    }
}


void signal_ignore() {
    printf("\n3. Normal signal queue - SIG_IGN\n");
    int pid;
    signal(SIGINT, handler2);

    err_msg = (char *)malloc(MAX_MSG_SIZE);
    if (err_msg == NULL) {
        printf("Error occur!\n");
        exit(-1);
    }

    strcpy(err_msg, "No errors yet.");

    if((pid = fork()) == 0) { // Child process
        pid = getppid();
        for(int i=0;i<10;i++) {
            printf("Sending: sig = %d \n", i);
            raise(SIGINT);
            sleep(1);
        }

        exit(0);
    }

    // Parent process -----------
    int end = 12;
    while(1) {
        printf("Signal received: error = %s, PID= %d, UID= %d\n", err_msg, pid_glob, uid_glob);
        sleep(1);
        if(end == 0) break;
        end--;
    }
}


int main( int argc, char * argv[] ) {
    signal_queue();
    signal_child();
    signal_ignore();

    return 0;
}

