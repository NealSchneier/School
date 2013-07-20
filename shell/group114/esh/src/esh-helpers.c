#include <obstack.h>
#include <stdlib.h>
#include <termios.h>


static void	give_terminal_to(pid_t pgrp, struct termios *pg_tty_state);
char * getStatus(int status);
pid_t getpgid(pid_t pid);
static void sigint_handler(int signo, siginfo_t *info, void *_ctxt);
static void sigstp_handler(int signo, siginfo_t *info, void *_ctxt);
char* getStatus(int status );
static void sigchld_handler(int signo, siginfo_t * info, void * _ctxt);
static void getPlugins(struct esh_pipeline *pipeC);
static void loop_pipeline(struct esh_command_line *cline, struct list *list_jobs);
static void loop_command(struct list_elem *e, struct esh_pipeline *cline);
static void nonPlugins(struct list_elem *e, struct esh_pipeline *pipeC);


static void exitFunction();//when exit command called
pid_t parentPID;//the parent pid
struct termios *terminalState;//the state saved initially
struct list list_jobs;//the list of jobs that are in excuting or stopped
int jobsID = 1;//tracks the number of current jobs
int exitCalled = 1;//determines exit has been called while background job is there or was recently added.

/* Loops through each pipeline and then creates the current
pipeline command from the command line. */
static void loop_pipeline(struct esh_command_line *cline, struct list *list_jobs)
{
	struct list_elem *e = list_begin (&cline->pipes);
	for (; e!= list_end (&cline->pipes); e=list_next(e))
	{
		struct esh_pipeline *pipe = list_entry(e, struct esh_pipeline, elem);//current pipeline
		loop_command(e, pipe);		
	}
}
/* Loops through each command that it is passed and determines the proper action based on the
command. For example, it determines if it is a plugin, a pipe, a builtin command or
or one of the already executable bash commands.
*/
static void loop_command(struct list_elem *e, struct esh_pipeline *pipeC)
{
	//struct list_elem *j = list_begin (&pipeC->commands);//current element from the pipeline
	int pluginsPresent = 0;//check of if it is a plugin
	/*pid_t pid; // current pid
	int status; // current status
	
	struct list_elem *pluginElement = list_begin(&esh_plugin_list);//get first plugin in the list
	struct esh_command *cmd = list_entry(j, struct esh_command, elem);//get the command that will be used to execute*/
	getPlugins( pipeC);
	//if there wasnt a plugin entered
	if (!pluginsPresent)
	{
		nonPlugins(e, pipeC);
		
	}
}


/*
the given function, it gives the terminal the given process.
*/
static void give_terminal_to(pid_t pgrp, struct termios *pg_tty_state)
{
    esh_signal_block(SIGTTOU);
    int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
    if (rc == -1){
        esh_sys_fatal_error("tcsetpgrp: ");
	}
    if (pg_tty_state){
        esh_sys_tty_restore(pg_tty_state);
	}
    esh_signal_unblock(SIGTTOU);
}

/*
this returns a pointer to characters for based on the int of the status of the process
*/
char* getStatus(int status )
{
	char *s;
	switch(status)
	{
		case 0: s = "Running";
		break;
		case 1: s = "Running";
		break;
		case 2: s = "Stopped";
		break;
		case 3: s= "Stopped";
	}
	return s;
}

/*
child handler for sigint. Catches the signal and manipulates how we need.
*/
static void sigint_handler(int signo, siginfo_t *info, void *_ctxt)
{
}
/*
child handler for sigtstp. Catches the signal and manipulates how we need.
*/
static void sigstp_handler(int signo, siginfo_t *info, void *_ctxt)
{
}
/*
this catches sigchld and waits for the process to complete so that it correctly
reaps the children so that there are no zombies.
*/
static void sigchld_handler(int signo, siginfo_t * info, void * _ctxt)
{
	pid_t pid;
	
	//this waits while all the child processes are being completed.
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
	{
		struct list_elem * e = list_end(&list_jobs);
		//goes through the jobs list.
		for (e = list_begin(&list_jobs); e != list_end(&list_jobs); e = list_next(e))
		{
			struct esh_pipeline * job = list_entry (e, struct esh_pipeline, elem);
			//removes the element from jobs list when the process has been compeleted.
			if (job->pgrp == pid)
			{
				list_remove(e);
				jobsID--;
				break;
			}
		}
	}
}
//the exit function called
static void exitFunction()
{
	//when there are background running jobs wont let you exit on the first attempt
	if (exitCalled == 0)
	{
		printf("exit\r\n");
		exitCalled = 1;
		}//when you exit and there are no backrunning jobs or exit has been entered twice in a row.
		else 
			if (kill(parentPID, SIGKILL) < 0)
			{
				printf("kill error");
				exit(1);
			}
}
//gets the plugins from the list
static void getPlugins(struct esh_pipeline *pipeC)
{
	struct list_elem *j = list_begin (&pipeC->commands);//current element from the pipeline
	int pluginsPresent = 0;//check of if it is a plugin
	struct list_elem *pluginElement = list_begin(&esh_plugin_list);//get first plugin in the list
	struct esh_command *cmd = list_entry(j, struct esh_command, elem);//get the command that will be used to execute
	//goes throught the plugin list and runs them
	while (pluginElement != list_end(&esh_plugin_list) && pluginsPresent != 1)
	{
		struct esh_plugin *plug = list_entry(pluginElement, struct esh_plugin, elem);
		if (plug->process_builtin == NULL)//if nothing entered
			continue;
		
		if (plug->process_builtin(cmd))//if the process is built in 
			pluginsPresent = true;
	
		pluginElement = list_next(pluginElement);//move to the next element in the list
		
	}
}
//runs most commands that are built in commands or other bash commands.
static void nonPlugins(struct list_elem *e, struct esh_pipeline *pipeC)
{
	struct list_elem *j = list_begin (&pipeC->commands);//current element from the pipeline
	pid_t pid; // current pid
	int status; // current status
	struct esh_command *cmd = list_entry(j, struct esh_command, elem);//get the command that will be used to execute
	
	char **p = cmd->argv;//the current command line and arguments
	parentPID = getpid();//used to track the current parent id
	/*
	jobs command, cycles throught the list of jobs and prints the correct output
	*/
	if (strcmp(*p, "jobs") == 0){					
		struct list_elem *temp;
		for (temp = list_begin(&list_jobs); temp != list_end(&list_jobs); temp= list_next(temp))
		{ 
			struct esh_pipeline *a = list_entry(temp, struct esh_pipeline, elem);
			struct list cmds = a->commands;
			struct list_elem *pElem = list_begin(&cmds);
			struct esh_command *pCmd = list_entry(pElem, struct esh_command, elem);
			char **para = pCmd->argv;//stores the parameter is command has one
			char **y = para;//stores the command
			para++;				
			/*
			prints out based on command
			*/
			if (*para != NULL)
				printf("[%d]+ %-24s (%s %s)\n", a->jid, getStatus(a->status), *y, *para);
			else 
				printf("[%d]+ %-24s (%s)\n", a->jid, getStatus(a->status), *y);
		}
		
	}
	/*
	the built in kill command. It will get the job id that is passed and then cycle through
	the jobs list until it finds the correct job id and then kills that process. Properly 
	reaps the children.
	*/
	else if (strcmp(*p, "kill") == 0){
		p++;
		if (*p != NULL)
		{
			int job_id = atoi(*p);//job id passed
			struct list_elem *temp = list_begin(&list_jobs);
			int status;
			int count =0;//check to make sure the loop doesnt go past the current size
			struct esh_pipeline *a;
			
			while (count < job_id && temp != list_end(&list_jobs))
			{
				a = list_entry(temp, struct esh_pipeline, elem);
				//if the jobs id match then it kills the process and removes it from 
				//the jobs list
				if (job_id == a->jid)
				{
					if (kill(a->pgrp, SIGKILL) < 0)
					{
						printf("kill error");
						exit(1);
					}
					list_remove(temp);
					jobsID--;
				}
				
				count++;
				temp = list_next(temp);
			}
			waitpid(-1, &status, WNOHANG);
		}
		else ;				
	}
	/*
	the fg command which brings a job to the foreground. if no parameters then the most recent non stopped
	job. If a parament is passed then it matches the job id to the list and brings it to the foreground.		
	*/
	else if (strcmp(*p, "fg") == 0){
		p++;
		//makes sure that the list isnt empty and when a job id number is passed
		if (*p != NULL && list_size(&list_jobs) > 0){
			int job_id = atoi(*p);//jobs id
			struct list_elem *e;
			int status;
			int a ;
			//goes through the list
			for (e = list_begin (&list_jobs); e !=list_end(&list_jobs); e = list_next(e))
			{
				struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);
				//matches the jobs and prints the correct info
				if (pipeline->jid == job_id)
				{
					a = pipeline->pgrp;
					struct list cmds = pipeline->commands;
					struct list_elem *pElem = list_begin(&cmds);
					struct esh_command *pCmd = list_entry(pElem, struct esh_command, elem);
					char **para = pCmd->argv;
					char **y = para;
					para++;
					if (*para !=NULL)
					{
						printf("%s %s\r\n", *y, *para);
					}
					else printf("%s\r\n", *y);
					//removes jobs from tlist
					//list_remove(e);
					//jobsID--;
				}
			}
			//this gives the terminal to the command and waits for the process to finish before
			//giving control back to terminal
			give_terminal_to(a, NULL);
			if (kill(a, SIGCONT) < 0 )
			{
				printf("kill error");
				exit(1);				
			}
			waitpid(a, &status,  WUNTRACED);
			give_terminal_to(getpgid(parentPID), terminalState);
			
		}
		//when a job is in the background and no parameter given
		else if (list_size(&list_jobs) > 0){
			//this goes into the command and gets the proper information to print out.
			int status;
			struct list_elem *temp = list_pop_back(&list_jobs);
			struct esh_pipeline *get = list_entry(temp, struct esh_pipeline, elem);
			struct list cmds = get->commands;
			struct list_elem *pElem = list_begin(&cmds);
			struct esh_command *pCmd = list_entry(pElem, struct esh_command, elem);
			printf("%s\r\n", *(pCmd->argv));
			//removes the element
			temp = list_pop_back(&list_jobs);
			jobsID--;
			//this next group continues the last process and gives it the terminal. 
			//when done executing it returns the terminal.
			int jidNew = get->pgrp;
			give_terminal_to(jidNew, NULL);
			if (kill(jidNew, SIGCONT) < 0)
			{
				printf("kill error");
				exit(1);			
			}
			waitpid(jidNew, &status,  WUNTRACED );
			give_terminal_to(getpgid(parentPID), terminalState);
		}	
	}
	/*
	the bg command backgrounds the selected process. 
	*/
	else if (strcmp(*p, "bg") == 0){
		p++;
		if (*p == NULL) // when no parameters given it just prints out what is running in the background
		{
			struct list_elem *temp = list_begin(&list_jobs);
			int count = 0;
			struct esh_pipeline *a;
			//goes through the list
			while (count < jobsID && temp != list_end(&list_jobs))
			{
				a = list_entry(temp, struct esh_pipeline, elem);
				count++;
				temp = list_next(temp);
			}
		}
		else {
			//when bg is passed with the job id number
			struct list_elem *temp = list_begin(&list_jobs);
			int count = 0;
			struct esh_pipeline *a;
			int job_id = atoi(*p);
			//goes throught the jobs list
			while (count != job_id && temp != list_end(&list_jobs))
			{
				a = list_entry(temp, struct esh_pipeline, elem);
				//continues the job that matches the job id number passed and sends it the 
				//continue signal if it is stopped.
				if (job_id == a->jid && a->status == STOPPED)
				{
					if (kill(a->pgrp, SIGCONT) < 0)
					{
						printf("kill error");
						exit(1);
					}
					break;
				}
				count++;
				temp = list_next(temp);
			}
			printf("pid %d\n", a->pgrp);
			printf("jid %d\n", a->jid);
		}
	}
	/*the stop command will stop a job when it is passed the job id number.
	
	*/
	else if (strcmp(*p, "stop") == 0)
	{
		p++;
		//this is the case when it has a job id number attached.
		if (*p != NULL)
		{
			int job_id = atoi(*p);
			struct list_elem *e;
			int a ;
			//goes through the jobs list
			for (e = list_begin (&list_jobs); e !=list_end(&list_jobs); e = list_next(e))
			{
				struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);
				if (pipeline->jid == job_id)//when the job id number matches
				{
					//gets the pgrp id and suspends that. It the changes the status and prints out
					//the needed output.
					a = pipeline->pgrp;
					if (kill(a, SIGTSTP) <0)
					{
						printf("kill error");
						exit(1);
					}
					pipeline->status= STOPPED;
					struct list cmds = pipeline->commands;
					struct list_elem *pElem = list_begin(&cmds);
					struct esh_command *pCmd = list_entry(pElem, struct esh_command, elem);
					char **para = pCmd->argv;
					char **y = para;
					para++;
					if (*para !=NULL)
					{
						printf("%s %s\r\n", *y, *para);
					}
					else printf("%s\r\n", *y);
				}
			}
		}		
	}
	/*
	This is the basic exit command. Allows you to exit the back running terminal.
	*/
	else if (strcmp(*p, "exit")==0)
	{
		exitFunction();
	}
	/*This is where file I/O, piping, and basic bash commands are run.
	*/
	else {
		int pCount = list_size(&pipeC->commands);//the number of commands in the pipe
		int pipe1[2];//a file descriptor
		pipe(pipe1);//piping the file descriptor
		if ((pid = fork()) < 0) //This is seperating into parent process+child
		{
			//if the fork failed
			printf("ERROR");
			exit(1);
		}
		//The child process will execute this next code. Not the parent.
		if (pid == 0 )
		{	
			setpgid(0,0);//makes the child process head of its own process group.
			esh_signal_sethandler(SIGINT, sigint_handler);//^c handler for child
			esh_signal_sethandler(SIGTSTP, sigstp_handler);//^z handler for child
			//if the number of commands is greater than 1 ie is a pipelined command.
			if (pCount > 1)
			{
				int i = 0;
				//forks the initially command and checks to make sure there isnt an error
				if ((pid = fork()) < 0)
				{
					printf("fork error");
					exit(1);
				}
				//goes through each of the piped commands
				for (;j !=list_end (&pipeC->commands); j = list_next(j) )
				{
					struct esh_command *cmd = list_entry(j, struct esh_command, elem);
					char **p = cmd->argv;
					//Checks which process is being run to make sure both functions are running different programs.
					if (pid != 0 && i == 1){ //The child that was called to run the next command
						
						dup2(pipe1[0], STDIN_FILENO);
						if (close(pipe1[0]) < 0)
						{
							printf("close error");
							exit(1);
						}
						if (close(pipe1[1]) < 0)
						{
							printf("close error");
							exit(1);
						}

						if (execvp(*p, p) < 0){
							printf("exec error");
							exit(1);
						}
					}
					else if (pid == 0 && i != 0){
						dup2(pipe1[1], STDOUT_FILENO);
						if (close(pipe1[0]) < 0 )
						{
							printf("close error");
							exit(1);
						}
						if (close(pipe1[1]) < 0)
						{
							printf("close error");
							exit(1);
						}
						
						if (execvp(*p, p) < 0){
							printf("exec error");
							exit(1);
						}
					}
					else if (pid == 0 && i == 0){//Redirect a file io the parent that called the fork
						dup2(pipe1[1], STDOUT_FILENO);
						if (close(pipe1[0]) <0)
						{
							printf("close error");
							exit(1);
						}
						if (close(pipe1[1]) < 0)
						{
							printf("close error");
							exit(1);
						}
						
						if (execvp(*p, p) < 0){
						   printf("exec error");
						   exit(1);
						}
					}
					else{  //If there's more then one
						i++;
						if (pCount != 2){
							
							if ((pid = fork()) < 0)
							{
								printf("fork error");
								exit(1);
							}								
						}
					}
				}
			}
			else 
			{
				//Takes the element from the list
				struct list_elem *k = list_begin(&pipeC->commands);
				struct esh_command *kcmd = list_entry(k, struct esh_command, elem);

				FILE *file = fopen(kcmd->iored_output, "r");
				//If there is an output specified
				if (kcmd->iored_output){
					//Appends to output if necessary
					if (kcmd->append_to_output){
						//Check to see if the file exists.
						if (!file){
							int outputFd = open(kcmd->iored_output, O_CREAT | O_WRONLY, S_IRUSR| S_IXUSR | S_IWUSR);
							dup2(outputFd, 1);
							dup2(outputFd, 2);
							if (close(outputFd) < 0)
							{
								printf("close error");
								exit(1);
							}	
							//Checks the execution of the command.
							if (execvp(*p, p) <0)
							{
								printf ("exec error");
								exit(1);
							}
						}
						else{ //Sets up the piping and creates it if it doesn't exist.
							int outputFd = open(kcmd->iored_output, O_APPEND | O_WRONLY, S_IRUSR| S_IXUSR | S_IWUSR);
							dup2(outputFd, 1);
							dup2(outputFd, 2);

							if (close(outputFd) < 0)
							{
								printf("close error");
								exit(1);
							}
							//Checks the execution of the command.
							if (execvp(*p, p) < 0)
							{
								printf ("exec error");
								exit(1);
							}
						}
					}
					else{
						//This is code that will always create the file.
						int outputFd = open(kcmd->iored_output, O_CREAT | O_WRONLY, S_IRUSR| S_IXUSR | S_IWUSR);

						dup2(outputFd, 1);
						dup2(outputFd, 2);
						if (close(outputFd) < 0)
						{
							printf("close error");
							exit(1);
						}

						//Checks the execution of the command.
						if (execvp(*p, p) <0)
						{
							printf ("exec error");
							exit(1);
						}
					}
				}
				//If there's an input specified
				if (kcmd->iored_input){
					if (!file){ //the file doesn't exist
						fprintf(stderr, "File doesn't exist!");
						exit(1);
					}
					else{ //The file does exist
						int inputFd = open(kcmd->iored_input, O_RDONLY, S_IRUSR | S_IWUSR);
						if (close(STDIN_FILENO) < 0)
						{
							printf("close error");
							exit(1);
						}
						dup2(inputFd, 0);

						if (close(inputFd) < 0)
						{
							printf("close error");
							exit(1);
						}

						//Checks the execution of the command.
						if (execvp(*(kcmd->argv), p) <0)
						{
								printf ("exec error");
								exit(1);
						}
					}
				}
				//when it isnt a piped or IO process
				if (execvp(*p, p) <0)
				{
					//printf ("exec error");
					exit(1);
				}
			}
		}
		else
		{
			//Closes pipes in the shell
			if (close(pipe1[0]) < 0)
			{
				printf("close error");
				exit(1);
			}
			if (close(pipe1[1]) < 0)
			{
				printf("close error");
				exit(1);
			}
			int newID = getpgid(pid);//gets the current process id
			//waits until the child process id has changed, which shoes that it has started
			while (newID == getpgid(getpid()))
			{
				newID = getpgid(pid);
			}
			list_remove(j);//removes the element from its list so it can be used by us
			//this allocates memory for our newly create pipeline which we then add to our list of jobs.
			struct esh_pipeline *now = 	(struct esh_pipeline *)malloc(sizeof(struct esh_pipeline));
			struct list commandsList;//the list to store the jobs that are in the pipeline
			list_init(&commandsList);
			list_push_back(&commandsList, j);//adds the commands to our pipeline.
			//this deep copies the parts from the removed element and the adds it to our allocated element.
			now->commands = commandsList;
			now->iored_input = pipeC->iored_input;
			now->iored_output = pipeC->iored_output;
			now->append_to_output = pipeC->append_to_output;
			now->bg_job = pipeC->bg_job;
			now->jid = jobsID;
			pipeC->jid = jobsID;
			pipeC->pgrp = parentPID;
			now->pgrp = newID;
			now->status = pipeC->status;
			//makes the status the correct output.
			if (now->bg_job == 1)
				now->status = BACKGROUND; 
			else 
				now->status = FOREGROUND; 
			now->saved_tty_state = *terminalState;
			pipeC->saved_tty_state = *terminalState;
			list_push_back(&list_jobs, &(now->elem));//adds our element to jobs list
			jobsID++;
			//if the child process is a background or foreground job.
			if (now->bg_job == 0)
			{
				//is a foreground job so command of the terminal is given.
				give_terminal_to(pid,  NULL );
				esh_signal_block(SIGCHLD);
				waitpid(pid, &status, WCONTINUED | WUNTRACED);
				esh_signal_unblock(SIGCHLD);
				give_terminal_to(parentPID, terminalState);
				/*
					This is executed if ^z is pressed of any jobs is suspended
				*/
				if (status == 5247)
				{	
					exitCalled = 0; 
					char *jobname = *p;
					now->status=STOPPED;//update status
					p++;
					//correctly format output.
					if (*p != NULL)
						printf("[%d]+ %s\t(%s %s)\n", now->jid, getStatus(now->status), jobname, *p);
					else 
						printf("[%d]+ %s\t(%s)\n", now->jid, getStatus(now->status), jobname);
				}
				else 
				{
					//if the jobs isnt suspended then it removes the foreground process form the jobs list
					//it also frees the malloc pipeline command
					list_pop_back(&list_jobs);
					free(now);
					jobsID--;
				}
			}
			else 
			{	
				//if a background process so it continues and activates the sigchild handler,
				//prints the correct output.
				esh_signal_sethandler(SIGCHLD, sigchld_handler);
				printf("[%d] %d\r\n", --jobsID, newID);
				jobsID++;
			}
		}
	}
}



