#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
             exit(EXIT_FAILURE))

#define HERE puts("***********************")

//MAX_BUFF must be in one byte range
#define MAX_BUFF 200

void create_m();

void usage(char * name)
{
    fprintf(stderr, "USAGE: %s t n r b\n", name);
    fprintf(stderr, "50<=t<=500\n");
	fprintf(stderr, "3<=n<=30 \n");
	fprintf(stderr, "0<=r<=100 \n");
	fprintf(stderr, "1<=b<=PIPE_BUF-6 \n");
    exit(EXIT_FAILURE);
}

int sethandler( void (*f)(int), int sigNo) 
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void sigchld_handler(int sig) 
{
    pid_t pid;
    for(;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0) return;
        if (pid < 0) 
        {
            if (ECHILD == errno) return;
            ERR("waitpid:");
        }
    }
}

void* c_work(void* voidData) 
{
	pthread_t mtid;
    printf("c starting PID:%d\n", getpid());
	create_m(&mtid);
    
    HERE;
    if (pthread_join(mtid, NULL) == 0) fprintf(stderr, "Joined with thread\n");
    HERE;
    
    printf("c exiting PID:%d\n", getpid());
    //exit(EXIT_SUCCESS);

    return NULL;
}

void* m_work(void* voidData) 
{
	printf("m starting PID:%d PPID:%d\n", getpid(), getppid());
	sleep(1);

    printf("m exiting PID:%d PPID:%d\n", getpid(), getppid());
	
    //exit(EXIT_SUCCESS);
    return NULL;
}

void create_m(pthread_t* mtid)
{
	
    if (pthread_create(mtid, NULL, &m_work, NULL) < 0) ERR("m Pthreadcreate");
    printf("m created with TID:%lu \n", *mtid);

    // switch (fork()) 
	// {
	// 	case 0:
			
	// 		m_work();

	// 		printf("m exiting PID:%d PPID:%d\n", getpid(), getppid());
	// 		exit(EXIT_SUCCESS);

	// 	case -1: ERR("m fork:");
	// }
}

void create_c_and_pipes() 
{
    int c = 2;
    pthread_t ctids[2];

    while (c) 
    {
        
        printf("creating c%d\n", c);
        pthread_create(&ctids[c-1], NULL, &c_work, NULL);
        printf("c%d created with TID:%lu \n", c, ctids[c-1]);
        
        
        // switch (fork()) 
        // {
        //     case 0:
                
        //         c_work();

		// 		while(wait(NULL) > 0);
		// 		printf("c exiting PID:%d\n", getpid());
        //         exit(EXIT_SUCCESS);

        //     case -1: ERR("c fork:");
        // }

		c--;
    }

    c = 2;

    HERE;

    while (c) 
    {
        if (pthread_join(ctids[c-1], NULL) == 0) fprintf(stderr, "Joined with thread\n");
        c--;
    }
}

int main(int argc, char** argv) 
{
    int t, n, r, b; 
    if (5 != argc) usage(argv[0]);
    
    t = atoi(argv[1]);
	n = atoi(argv[2]);
	r = atoi(argv[3]);
	b = atoi(argv[4]);
    if (t<50 || t>500) 			usage(argv[0]);
	if (n<3  || n>30) 			usage(argv[0]);
	if (r<0  || r>100) 			usage(argv[0]);
	if (b<1  || b>PIPE_BUF-6) 	usage(argv[0]);

	printf("t=%d, n=%d, r=%d, b=%d\n", t, n, r, b);
    
    if (sethandler(SIG_IGN, SIGINT)) ERR("Setting SIGINT handler");
    if (sethandler(SIG_IGN, SIGPIPE)) ERR("Setting SIGINT handler");
    if (sethandler(sigchld_handler, SIGCHLD)) ERR("Setting parent SIGCHLD:");
    
    create_c_and_pipes(); 
    
    //parent_work();
    
    while(wait(NULL) > 0);
    return EXIT_SUCCESS;
}
