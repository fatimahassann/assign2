#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <stdbool.h>

#define BUFFER_LEN 1024
int flag_pipe=0;  /* | */
int flag_right=0; /* < */
int flag_left=0; /* > */
int flag_equal=0; /* = */
int loc_left=0; /* the location of > in the command */
int loc_right=0; /*location of < in the command */
int loc_pipe[50]; /*location of pipes (max 50 pipes )*/
int flag2=0;

/* creating a struct for each cell */
struct cell
{
char array[100];
int flag;
};


struct cell storing1[100]; /* array for elements before the = sign */
struct cell storing2[100]; /* array for elements after the = sign */



size_t read_command(char * cmd)
{
	if (!fgets(cmd, BUFFER_LEN, stdin))
		return 0;
	size_t length = strlen( cmd );
	if ( cmd[length -1] == '\n')
		cmd[length-1] = '\0';
	return strlen(cmd);

}

int build_args(char * cmd, char ** argv)
{
	char *token;
	token=strtok(cmd, " ");
	int i=0;
	int counter=0;
	while (token !=NULL)
	{
		argv[i]=token;
	/* changing the corresponding flag if we have the special character | or < or > or = */
		if (strcmp(token,"|")==0)
		{	flag_pipe++;
			loc_pipe[counter]=i;
			counter++;
			flag2=1;
		}

		if(strcmp(token,"<")==0)
		{	flag_right=1;
			loc_right=i;
		}

		if(strcmp(token,">")==0)
		{	flag_left=1;
			loc_left=i;
		}
		
		if(strcmp(token,"=")==0)
			flag_equal=1;

		token=strtok(NULL, " ");
			i++;
	}
	argv[i]=NULL;
	return i;
}

void set_program_path(char * path, char * bin, char * prog)
{
	memset(path,0,1024);
	strcpy (path, bin);
	strcat (path, prog);
	int i=0;
	for ( i=0; i< strlen(path); i++)
		if (path[i]=='\n')
			path[i]='\0';
}

/*function to handle exit or ctrl d */
void exit_fun(char* argv[100])
{
	char* cmd=argv[0];
	
	if(cmd == "exit" )
		exit(0);
	
	/*handling the ctrl D */
	char s[100];
	s[0]=argv[0];
	char* D= strchr(s[0], 4);
	if (D)
		exit(0);
}


/*handle directories */

void directories(char* argv[100])
{
	/* set home directory */
	char* home = getenv("HOME");
	/*set the destination directory by taking the second element in the array */
	char* dest=argv[1];
	int i; 

		if(dest=="..")
			chdir(home);
		else
			i=chdir(dest);

		if(i==-1)
		{
			printf("No directory exists with this name \n");
		}


}



void creating_storing_array ()
{
	int i;
	i=0;
	int j;
	j=0;
	for (i=0; i<100; i++)
	{
		struct cell c1;
		struct cell c2;	
	
		storing1[i]=c1; 
		storing2[i]=c2;
	
		storing1->flag=0;
		storing2->flag=0;
	}	

}
void piping(char* cmd_before[100], char* cmd_after[100])
{
int pfd[2];

int pid=fork();
pipe(pfd); 
if(pid==-1)
	printf("pipe error");

if(pid==0) /*first child */
{	
	dup2(pfd[1],1);
	close(pfd[0]);
	close(pfd[1]); 
	execvp(cmd_before[0],cmd_before);
}

int pid2=fork(); 

if(pid2==-1)
	printf("pipe error");

if(pid2==0) /*second child */
{
	dup2(pfd[0],0);
	close(pfd[1]);
	close(pfd[0]);
	execvp(cmd_after[0], cmd_after);
}
}

int main()
{
	char line[BUFFER_LEN];
	char* argv[100];
	char* bin= "/bin/";
	char path[1024];
	int argc;

	creating_storing_array(); 


	while (1)
	{	/* printing out the current directory */
		char cwd[FILENAME_MAX];
		getcwd(cwd, sizeof(cwd));
		printf("Current directory: %s:\n",cwd); 

		printf("My shell >>");
		if (read_command(line)==0)
		{
			printf("\n");
			break;
		}

		if (strcmp(line, "exit")==0)
		{	break;
			exit_fun(argv);
		}	
	
		argc=build_args(line, argv);
		set_program_path(path, bin, argv[0]);
		char* filename_dest;
		char* filename_src;

		/* hadling if we have echo */
		if (strcmp(argv[0],"echo")==0) 
		{	
			int i;
			i=0; 
			int num;
			num=0;
			if(argv[1][0]=='$')
			{ /* printing element from the 2 restored variables */
				/* adding  a $ at the begining of the string to compare later */
				char r[50];
					/*comparing until we find the right element stored */
				while(num!=100 && i!=1) 
				{	r[0]='\0';
					strcat(r,"$");
					strcat(r,storing1[num].array); 

					if(strcmp(r,argv[1])==0)
					{
						printf("%s",storing2[num].array);
						putchar('\n');
						i=1;
					}
					else
						num ++;			
				}


			}
		
		}
		int pfd[2];
		char* cmd_before[100];
		char* cmd_after[100];
		/*handle piping */
		if(flag_pipe!=0)
		{
			int i;
			i=0;
			int counter;
			counter=0;
			int num;
			num=0;
			if(flag_pipe=1)
			{ /* seperating the command into 2 comands: one before and after | */
				while(i!=loc_pipe[0])
				{	
					cmd_before[i]=argv[i];
					i++;	
				}
				i++;
				while(argv[i]!=NULL)
				{
					cmd_after[counter]=argv[i];
					i++;
					counter++;
				}
				piping(cmd_before,cmd_after);
			
			}
			else /* if we have more than 1 pipe */
			{
				while(flag_pipe!=0)
				{
					int j=0;
					int k=1;
					int c=0;
					while(j!=loc_pipe[k])
					{
						j++;
						cmd_before[j]=argv[j];
					}
					j++;
					while(argv[j]!=NULL)
					{	cmd_after[c]=argv[j];
						j++;
						c++;
					}
					piping(cmd_before,cmd_after);
					k++;
					flag_pipe--;
				}
			}
		}

		/* handle < redirection */
		if(flag_right==1)
		{
			filename_src=argv[loc_right+1];
			int i=0;
		/* adjusting so that we can remove the < */
			for ( i = loc_right; argv[i-1]!=NULL;i++)
				argv[i]=argv[i+2];

			
		} 
		/* handle > redirection */

		if(flag_left==1)
		{	
			filename_dest=argv[loc_left+1];
			int i=0;
		/* adjusting so that we can remove > filename_dest from the file we created */
			for ( i=loc_left;argv[i-1]!=NULL; i++)
				argv[i]=argv[i+2];
		}

		/* handle = */
		if(flag_equal==1)
			/* i is the first empty space in the storing array */
		{ 
			int i;
			i=0;
			int count=0;
			int f=0;
			/* finding the first empty space in the array */
			while (storing1[i].flag != 0 && i<100)
			{
				i++;
			}
			/* if the user stored more than 100 elements */			
			if ( i == 100)
			{	printf("too many elements are stored, restart the program"); 
				return;
			}
			/* case if we have x=$y (something stored already) */
			if(argv[2][0]=='$')
			{
				char a[50];
				while(count!=100 && f!=1)
				{	/* comparing with elements stored until we find y */
					a[0]='\0';
					strcat(a,"$");
					strcat(a,storing1[count].array);
					
					if(strcmp(argv[2],a)==0)
					{
						f=1;
						const char* one;
						one=argv[0];
						const char* two;
						two=storing2[count].array;


						strcpy(storing1[i].array,one);
						strcpy(storing2[i].array,two);

						storing1[i].flag=1;
						storing2[i].flag=1;

					}
					else
						count ++;

				}
			}
			else
			{
			/*storing the new value in the storing arrays */
				
				const char* first;
				first=argv[0];
				const char* second;
				second=argv[2];	
				strcpy(storing1[i].array,first);
				strcpy(storing2[i].array,second);

	
			
				storing1[i].flag=1;
				storing2[i].flag=1;

			}	
		}



		/*handling if we have cd and calling the function */
		 if(strcmp(argv[0],"cd")==0)
		{	
			directories(argv);
		}
		else
		{	
			int pid=fork();
			if ( pid == 0)
			{	if(flag_right==1)
				{
					freopen(filename_src,"r",stdin); 
				} 
				if (flag_left==1)
				{
					freopen(filename_dest, "w+", stdout);
				}
			

				execve(path, argv, 0); 
			
				fprintf(stderr, "Child process could not do xecve\n");
			}
		       	else
			wait(NULL);
		}
	
		/* reseting the flags */
		flag_pipe=0;
		flag_right=0;
		flag_left=0;
		flag_equal=0;
									
	}	

	/* clearing the storing arrays */
	int c,d;
	c=0;
	for(c=0;c<100;c++)
	{
		storing1[c].flag=0;
		storing2[c].flag=0;
	}
	return 0; 
}

		





