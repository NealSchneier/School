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

#Removes the file to start fresh"
c.sendline("clear");
c.sendline("rm test_append.txt");

#Checks what's in the file
c.sendline("echo hello >> test_append.txt");
c.sendline("cat test_append.txt");
assert c.expect_exact("hello") == 0, "hello is present";

#Add in more things
#assert c.expect_exact("test_append.txt") == 0, "The test output is present");
c.sendline("echo String2 >> test_append.txt");
c.sendline("cat test_append.txt");
#assert c.expect_exact("String2") == 0, "String2 present");

#Check and see if the other words are still in the file.
c.sendline("cat test_append.txt");
#assert c.expect_exact("string1") == 0, "string1 still present");
#assert c.expect_exact("test_append.txt") == 0, "The test output is still present");

shellio.success()
