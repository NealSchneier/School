/*
 * esh - the 'pluggable' shell.
 *
 * Developed by Godmar Back for CS 3214 Fall 2009
 * Virginia Tech.
 */

#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "esh.h"
#include "esh-sys-utils.h"
#include "esh-helpers.c"

struct termios *termState;//the state saved initially
struct list listjobs;//the list of jobs that are in excuting or stopped
int jobID = 1;//tracks the number of current jobs
int exitCall = 1;//determines exit has been called while background job is there or was recently added.


static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
        " -h            print this help\n"
        " -p  plugindir directory from which to load plug-ins\n",
        progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that 
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */

static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;
    struct list_elem * e = list_begin(&esh_plugin_list);

    for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL)
            continue;

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL)
        prompt = strdup("esh> ");

    return prompt;
}

/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell =
{
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */ 
    .parse_command_line = esh_parse_command_line /* Default parser */
};

int
main(int ac, char *av[])
{   
    int opt;
    list_init(&esh_plugin_list);
    list_init(&list_jobs); //create our jobs list.
	
	terminalState = esh_sys_tty_init();//get the current state of the terminal.

    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;
			

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }
    esh_plugin_initialize(&shell);
	
    /* Read/eval loop. This loop runs everytime that enter is pressed */
    for (;;) {
        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
		
        char * cmdline = shell.readline(prompt);        
		free (prompt);

        if (cmdline == NULL)  /* User typed EOF */
            break;

        struct esh_command_line * cline = shell.parse_command_line(cmdline);
        //free (cmdline);
        if (list_empty(&cline->pipes))    /* User hit enter */
            continue;
		
		esh_command_line_print(cline);
		//evaluates the pipeline and then frees it.
		loop_pipeline(cline, &listjobs);
		esh_command_line_free(cline);
		
	}
	return 0;
}





