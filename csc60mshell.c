/*Authors: Cuong Su, Tan Dao, Kandy Ho, Christopher Sok
 *Date of Release: November 23, 2015
 *Description:
 *This program is a basic C minishell that is an extension of lab 4's
 *minishell. Added capabilities are that it can run processes in the background
 *via &. It also will not exit the shell when typing CTRL + C. However, CTRL + C
 *will close a foreground process, but not a background process. Jobs command will
 *list the background processes. When a background job is finished, it will display
 *done. As is, most functionalities work as intended; however there are no error
 *checks, so there are a few bugs.
 */

/*Problems to solve:
 *[X] Ctrl + C to not close shell
 *[X] Ctrl + C to not close bg process
 *[X] If there is a fg process running and user tries to ctrl +C, kill process
 *[X] If there are bg processes still running and user tries to exit, notify them that there are bg processes still running.
 *[X] put process into background if it has &
 *[X] jobs command
 *[X] when a background job is removed from the job_array, display done
 *[X] when a background process is done, display done
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAXLINE 80
#define MAXARGS 20
int i;
int y = 1;
int arrayCount = 0;
int status;
bool exitcheck;

struct temp{
int Pid;
};
struct temp tempjobs[20];

/* make a struct holding the information about jobs */
struct job_array{
  int process_id; // process id
  char command[80]; // previous command with & removed.
  int job_number; // job number
};
struct job_array job[20];
/* ----------------------------------------------------------------- */
/*                  I/O Redirection checks and execvp                */
/* ----------------------------------------------------------------- */

void process_input(int argc, char **argv) {
  int i;
  int j;
  int count;

  if(strcmp(argv[0], ">") == 0){
    printf("ERROR: No/invalid command.\n");
    exit(0);
  }

  if(strcmp(argv[0], "<") == 0){
    printf("ERROR: No/invalid command.\n");
    exit(0);
  }

  if(argc == 2){
    if(strcmp(argv[1], ">") == 0){
       printf("ERROR: No redirection file specified.\n");
       exit(0);
    }
  }

  if(argc == 2){
    if(strcmp(argv[1], "<") == 0){
       printf("ERROR: No redirection file specified.\n");
       exit(0);
    }
  }

  for(j=0; j < argc; j++){
    if(strcmp(argv[j], "<")==0){
      count++;
      if(count >= 2){
        printf("ERROR: Cannot have two input redirects on one line.\n");
        exit(0);
      }
    }
  }

  for(i=0; i < argc; i++){
    if(strcmp(argv[i], ">")== 0){
        int fid = open(argv[i+1],O_CREAT|O_RDWR|S_IRUSR|S_IWUSR);
        dup2(fid, 1);
        close(fid);
        argv[i] = NULL;
    }else if(strcmp(argv[i], "<") ==0){
        int fid = open(argv[i+1], O_RDWR|O_CREAT|S_IRUSR|S_IWUSR);
        dup2(fid,0);
        close(fid);
        argv[i] = NULL;
    }
  }
  execvp(argv[0],argv);
}
/* ----------------------------------------------------------------- */
/*                  parse input line into argc/argv format           */
/* ----------------------------------------------------------------- */

int parseline(char *cmdline, char **argv){
  int count = 0;
  char *separator = " \n\t";
  argv[count] = strtok(cmdline, separator);
  while ((argv[count] != NULL) && (count+1 < MAXARGS)) {
   argv[++count] = strtok((char *) 0, separator);
  }
  return count;
}

/* ----------------------------------------------------------------- */
/*                  Signal comparison                                */
/* ----------------------------------------------------------------- */

/* displays Done when background process is finished */
void signalHandler(int signal){
  pid_t pid;
  pid = wait(NULL);
  y=0;
  while(y<=arrayCount+1){
    if(job[y].process_id == pid){
      printf("\n[%d] Done       [%s]\n",job[y].job_number,job[y].command);
    job[y].process_id = -1;
    } 
  y++;
  }
} 

/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */

int main(void){
  char cmdline[MAXLINE];
  char* newcmdline;
  char *argv[MAXARGS];
  char *cwd;
  char buff[MAXLINE+1];
  int argc;
  bool background;
  pid_t pid;
  signal(SIGCHLD,signalHandler);

  /* make it so CTRL + C is ignored throughout the whole shell. */
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  sigemptyset( &act.sa_mask );
  act.sa_flags = 0;
  sigaction( SIGINT, &act, 0 );

  for(i=0;i<10;i++) {
    printf(" csc60mshell > ");
    fgets(cmdline, MAXLINE, stdin);

  /* Call parseline to build argc/argv: argc/argv parameters declared above */
    argc = parseline(cmdline, argv);

  /* If user types in nothing, go to top of loop. */
    if(argc == 0) continue;

  /* If user enters "cd dir", change to "dir" directory.    */
  /* If directory does not exit, print error.               */
  /* Else if user types only cd, change to home directory.  */
    if(strcmp(argv[0],"cd") == 0){
      if(argc == 1){
        char *temp = getenv("HOME");
        chdir(temp);
      }else if(argc == 2){
        if(chdir(argv[1]) == 0){

        }else{
          perror(argv[1]);
        }
      }


    /* If user types in "pwd", print the current working directory.*/
    }else if(argc == 1 && strcmp(argv[0],"pwd") == 0){
      cwd = getcwd( buff, 256 );
      printf("%s\n", cwd );

    /* If user types in "exit", exit the program.  */
    }else if(argc == 1 && strcmp(argv[0],"exit") == 0){
      exitcheck = true;
      for(i=1;i<=arrayCount;i++){
        if(job[i].process_id != -1){
          exitcheck = false;
        }
      }
      if(exitcheck == true){
        exit(0);
      }else{
        printf("There are still background processes!\n");
        continue;
        }

    /* if user types in "job", display information about running background jobs */
    }else if(argc == 1 && strcmp(argv[0],"jobs") == 0){
      for(i=1;i<=arrayCount;i++){
        if(job[i].process_id != -1){
          printf("[%d]  Running         ",job[i].job_number);
          printf("%s\n",job[i].command);
        }
       else if(job[i].process_id == -1){
          printf("[%d]  Done            ",job[i].job_number);
          printf("%s\n",job[i].command);

       }
     }
     /* Checks whether or not the argument ends in an &. If it does, remove &
     * and make argv[2] null.
     */
    }
    background = ((strcmp(argv[argc-1], "&")) == 0);
    if(background == true){
      newcmdline = malloc(strlen(argv[0]) + strlen(argv[1]) + 1);
      if(argv[2] != NULL) {
        strcat(strcpy(newcmdline,argv[0])," ");
        strcat(newcmdline, argv[1]);
      }else if(argv[2] == NULL){
        strcat(newcmdline, argv[0]);
      }
        arrayCount++;
        job[arrayCount].job_number = arrayCount;
        argv[argc-1] = NULL;
        argc--;
    }

    /* fork off a process to make child and parent processes. */
    pid = fork();
    /* Child process */
    if(pid == 0 && background == false){
      /* make CTRL + C back to default, so that it can close foreground processes */
      act.sa_handler = SIG_DFL;
      sigemptyset( &act.sa_mask );
      act.sa_flags = 0;
      sigaction( SIGINT, &act, 0 );
      process_input(argc, argv);
      exit(0);
    }else if(pid == 0 && background == true){
      setpgid(pid,0);
      act.sa_handler = SIG_IGN;
      sigemptyset( &act.sa_mask );
      act.sa_flags = 0;
      sigaction( SIGINT, &act, 0);
      process_input(argc,argv);
    }else if(background == false){
      signal(SIGCHLD, SIG_IGN);
      waitpid(pid,&status,0);
      signal(SIGCHLD,signalHandler);
    }else{
      /*Parent process */
      if(background){
        /* Update the job array */
        job[arrayCount].process_id = pid;
        strcpy(job[arrayCount].command, newcmdline );
        printf("[%d] %d\n",job[arrayCount].job_number, job[arrayCount].process_id);
      }
      background = false;
    }
  }
}



