#include "config.h"

int shmidChild;
char *shmStrChild;
int childIds = 1;
int n;
enum state { idle, want_in, in_cs };

void signalHandleChild()
{
	shmdt(shmStrChild);
	shmctl(shmidChild, IPC_RMID, NULL);
	exit(0);
}

void fileWrite(int id)
{
 	FILE * fptr;
        fptr = fopen("data.txt","a");

        if(fptr==NULL)
	{
            printf("Cannot open the file");
            return ;
        }
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	fprintf(fptr,"%s File modified by process number %d",asctime(tm),id);
	fclose(fptr);
}

void logEntry(int id, char *str)
{
	char filename[50];
	snprintf(filename, 50, "logfile.%d", id);

	FILE * fptr;
	fptr = fopen(filename,"a");
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	fprintf(fptr,"%s Critical Section at %s",str,asctime(tm));
	fclose(fptr);
}

void childProcess()
{
	signal(SIGINT,signalHandleChild);
        shmidChild = shmget(SHMKEY,SHMSIZE,0666|IPC_CREAT);
        shmStrChild = shmat(shmidChild,(void*)0,0);
        printf("child %d Created\n",childIds);
        int count =5;
        while(1)
	{
		if (shmStrChild[childIds - 1] == '*')
		{
			printf("Child: %d\n", childIds);
			shmStrChild[childIds - 1] = '@';

			for (int itr = 0; itr < count; itr++)
			{
				int turnId = shmget(SHMTURN, sizeof(int), 0606|IPC_CREAT);
				int flagId = shmget(SHMFLAG, sizeof(enum state) * n, 0606|IPC_CREAT);

				int * turn = (int *) shmat(turnId,0,0);
                		enum state * flag = (enum state *) shmat(flagId,0,0);

				int j; //local to each process
				int i = childIds - 1;
				//not sure if this even works, but I just plugged it in from the notes, NEED TO TEST
				do
				{
					flag[i] = want_in; // Raise my flag
					j = *turn; //Set local variable 
					while (j != i) 
						j = (flag[j] != idle) ? *turn : (j + 1) % n;
					//Declare intention to enter critical section
					flag[i] = in_cs;
					//Check that no one else is in critical section
					for (j = 0; j < n; j++)
						if ((j != i) && (flag[j] == in_cs))
							break;
				}
				while ((j < n) || (*turn != i && flag[*turn] != idle));

				//Assign turn to self and enter critical section
				*turn = i;
				srand(time(0));

				//sleep between 1 and 4 seconds
				int randomSleepTimeBefore  = rand()%4;
                		int randomSleepTimeAfter  = rand()%4;
                		logEntry(childIds, ENTER);

				sleep(randomSleepTimeBefore);
				fileWrite(childIds);
				sleep(randomSleepTimeAfter);

				logEntry(childIds, EXIT);
				
				//Exit section
				j = (*turn + 1) % n;
				while (flag[j] == idle)
				j = (j + 1) % n;
				//Assign turn to next waiting process; change own flag to idle
				*turn = j;
				flag[i] = idle;
			}
		}
	}
}

int shmidParent;
char * shmStrParent;

void signalHandle(){
	shmdt(shmStrParent);
	shmctl(shmidParent,IPC_RMID,NULL);
	exit(0);
}

int main(int argc, char ** argv)
{

	signal(SIGINT,signalHandle);
	if(argc <4){
		printf("Not Enough Arguments");
        exit(0);
    	}

	int ss =  atoi(argv[2]);
	n =  atoi(argv[3]);

	if(n > MAXCHILD){
		printf("Max allowed slave process is %d",MAXCHILD);
        n = MAXCHILD;
    	}

	int *cIds = (int *)malloc(sizeof(int)*n);

	// n forks
	int child;
	for(int i=0;i<n;i++){
		child = fork();
		if(child==0){
            		//child area
            		childProcess();
        	}
        	else
		{
            		childIds++;
            		cIds[i]=child;
        	}
    	}

	//shared memories
	shmidParent = shmget(SHMKEY,SHMSIZE,0666|IPC_CREAT);    
    	shmStrParent = shmat(shmidParent,(void*)0,0);    
    	char at [20];
    	for(int i=0;i<MAXCHILD;i++)
        	shmStrParent[i]=at[i]='@';

   	char  input[100];

	//using while loop to test here, will remove and clean up after
	while (1)
	{
		if (strncmp(shmStrParent, at, n) == 0)
		{
			scanf("%[^\n]s",input);
			//check if slave number is less than n
			if (strlen(input) > 1) 
			{
				char *t1 = strtok(input, " ");
				if (t1 != NULL && (strcmp(t1, "slave") == 0))
				{
					t1 = strtok(0, " ");
					int sn = atoi(t1);
					if (sn <= n)
					{
						shmStrParent[sn - 1] = '*';
					}
					else
					{
						printf("Enter number between 1 and %d\n", n);
					}
				}
			}
			getchar();
		}
	}
	return 0;
} 





























