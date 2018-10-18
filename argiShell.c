/*
	Despina - Ekaterini Argiropoulou
		AEM: 8491
	Project of 7th semester 2017-2018
	Operating Systems	05.02.2018
	
	Reference:
		S. Brennan, «Tutorial - Write a Shell in C,»  https://brennan.io/2015/01/16/write-a-shell-in-c/
		R.-R. J. E. Gustavo A. Junipero, «Chapter5-WritingYourOwnShell,»  https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf.
*/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LSH_RL_BUFSIZE 512	//max number of characters
#define LSH_TOK_BUFSIZE 64	
#define NUM_COM 200	//max number of commands

void loop_inter(void);
char *read_line(void);
char **split_line(char *line);
int execute(char **args);
int launch(char **args);
int lsh_exit(char **args);
int lsh_cd(char **args);
int num_builtins();
int (*builtin_func[]) (char **);
char *builtin_str[] = {
  "cd",
  "quit"
};
void loop_batch(char *a);
char *read_file(FILE *fp);

/*
     main:
	depends on args
	interactive or batch mode or exit
*/
int main(int argc, char **argv)
{
  if (argc == 1) {
    loop_inter();
  } else if (argc == 2) {
    printf("batchfile code\n");
    loop_batch(argv[1]);
  } else {
    printf("Problem in args\n");
  }
  return EXIT_SUCCESS;
}
/*
     loop_inter:
	read, split commands, split args, repeat
*/
void loop_inter(void)
{
  char *line;
  char **args;
  int status;
  char *tokcom, **command = malloc(NUM_COM * sizeof(char*));
  do {
    printf("Argiropoulou_8491> ");	//prompt
    int p = 0;
    line = read_line();	//store in line the string from buffer
    tokcom = strtok(line,";&&");	//seperate commands
    while(tokcom != NULL) {
      command[p] = tokcom;
      p++;
      if(p == NUM_COM) {
        printf("only the first %d commands will be executed\n", NUM_COM);
        break;
      }
      tokcom = strtok(NULL, ";&&");
    }
    for( int i=0; i < p; i++) {
      args = split_line(command[i]);	//in every command, seperate its arguments
      status = execute(args);
      if(!status) break;
    }
    free(line);
    free(args);
  } while (status);
}
/*
     read_line:
	in interactive mode, read from keyboard
	return it
*/

char *read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF) {
      exit(EXIT_SUCCESS); 
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize) {	//check
        printf("max 512 char\n");
        exit(EXIT_FAILURE);	//exceed max num of character
    }
  }
}
/*
     split_line:
	seperate commands,  ; or &&
	return them
*/
char **split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  token = strtok(line, " ");
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
                free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " ");
  }
  tokens[position] = NULL;
  return tokens;
}

int execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    printf("An empty command was entered in interactive mode\n");
    return 1;
  }

  for (i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {	//check if command is one of the "special ones"
      return (*builtin_func[i])(args);
    }
  }

  return launch(args);	//else continue in launch
}

int launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork(); 
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {	//execute args parallel to this program
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);	//wait until completion
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int lsh_exit(char **args)
{
  return 0;	//exit == quit
}

int lsh_cd(char **args)
{
  if (args[1] == NULL) {	//special checks if cd
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}
//special functions
int (*builtin_func[]) (char **) = {
  &lsh_cd,	
  &lsh_exit
};
/*
     batch mode:
	read, split commands, split args, execute, ends
*/

void loop_batch(char *a)
{
  FILE *fp;
  char *line, *name;
  char **args;
  int status;
  char *tokcom, **command = malloc(NUM_COM * sizeof(char*));
    printf("Argiropoulou_8491> ");	//prompt
    fp = fopen(a,"r");
    if (fp == NULL) {
     printf("Error to open batchfile\n");
    } else {
      int p = 0;
      line = read_file(fp);
      fclose(fp);
      tokcom = strtok(line,";&&");	//seperate commands
      while(tokcom != NULL) {
        command[p] = tokcom;
        p++;
        if(p == NUM_COM) {
          printf("only the first %d commands will be executed\n", NUM_COM);
          break;
        }
        tokcom = strtok(NULL, ";&&");
      }
      for( int i=0; i < p; i++) {	//for each command, seperate its args
        args = split_line(command[i]);
        status = execute(args);
        if(!status) break;
      }
    }
    free(line);
    free(args);
}


char *read_file(FILE *fp)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * 1024);
  char c;
  int l=0;
  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = fgetc(fp);
    if (c == EOF) {
      return buffer;
    } else if (c == '\n') {
      buffer[position] = ';';
      l = 0;	//reset l flag
    } else {
      buffer[position] = c;
      l++;	//counter of num char in every line
    }
    position++;
    if (l >= bufsize) {
        printf("max 512 char in every line\n");
        exit(EXIT_FAILURE);	//brake if line has more than 512 char
    }
  }
}



