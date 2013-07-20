#!/usr/bin/python
#
# Block header comment
#
#
import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile)
atexit.register(force_shell_termination, shell_process=c)

#Set up to start a single pipe test.
c.sendline("echo hello | cat");
assert c.expect_exact("hello") == 0, "hello was printed";
c.sendline("ls > 1_pipe_test.txt");
c.sendline("ls");

#chexking to see if the file is present after adding it.
assert c.expect_exact("1_pipe_test.txt") == 0, "file present";
c.sendline("ls | cat");
assert c.expect_exact("1_pipe_test.txt") == 0, "file present in piping";


shellio.success()
