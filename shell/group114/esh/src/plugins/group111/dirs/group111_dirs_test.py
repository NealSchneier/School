#!/usr/bin/python
#
# Test file for dirs plugin.
#
#
import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check


#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	os.rmdir(folder1)
	os.rmdir(folder2)
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

c.timeout = 2

# the folders to use for testing
folder1 = def_module.folder + "group111_dirs_test1"
folder2 = def_module.folder + "group111_dirs_test2"

# make the directories we are going to need to ues for the tests
os.mkdir(folder1)
os.mkdir(folder2)

# move cursor past all plugin load messages
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt" 

c.sendline("dirs")

# go past the line we just sent
c.readline()
startingDir = c.readline()

startingDir = startingDir.strip()

# check current directory which should be the starting directory now
c.sendline("pwd")
assert c.expect_exact(startingDir + "\r\n") == 0, "Directory not correct"

# make sure excution ends and prompt is back
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("pushd " + folder1)

newDir1 = startingDir + "/" + folder1
newDir2 = startingDir + "/" + folder2

# move past prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# check the current directory of the shell
c.sendline("pwd")

# make sure the current directory was changed
assert c.expect_exact(newDir1 + "\r\n") == 0, "Current directory was not changed to pushed directory"

# swap top two directories
c.sendline("pushd")

# check current directory which should be the starting directory now
c.sendline("pwd")
assert c.expect_exact(startingDir + "\r\n") == 0, "Directory not changed back to original"

# check output of dirs with -p and -v switches
c.sendline("dirs -p")
c.readline()
firstDir = c.readline().strip()
secondDir = c.readline().strip()

assert firstDir == startingDir and secondDir == newDir1, "dirs -p output incorrect"

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("dirs -v")
c.readline()
firstDir = c.readline().strip()
secondDir = c.readline().strip()

assert firstDir == ("0 " + startingDir) and secondDir == ("1 " + newDir1), "dirs -v output incorrect"

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# add third directory
c.sendline("pushd " + folder2)

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# move to folder1 by rotating 2 to the left
c.sendline("pushd +2")

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# check current directory which should be folder1 now
c.sendline("pwd")
assert c.expect_exact(newDir1 + "\r\n") == 0, "Directory not correct"

# move to folder2 by rotating 2 to the right
c.sendline("pushd -1")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# check current directory which should be folde2 now
c.sendline("pwd")
assert c.expect_exact(newDir2 + "\r\n") == 0, "Directory not correct"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# pop middle directory which is folder1 and see that only 2 left are folder2 and the starting dir
c.sendline("popd -0")

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("dirs -p")
assert c.expect_exact(newDir2 + "\r\n") == 0, "Could not see newDir2 in dir list"
assert c.expect_exact(startingDir + "\r\n") == 0, "Could not see startingDir after newDir2"

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# put folder1 back in list
c.sendline("pushd")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"
c.sendline("pushd " + folder1)
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# pop first directory which is the folder2
c.sendline("popd +0")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# current directory should be starting directory
c.sendline("pwd")
c.expect_exact(startingDir + "\r\n") == 0, "Current dir was not starting directory"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# pop top (folder1) off and final current directory should be newDir2
c.sendline("popd")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"
c.sendline("pwd")
c.expect_exact(newDir2 + "\r\n") == 0, "Current directory was not newDir2"

# move past prompt 
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

#exit
c.sendline("exit");
assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"

shellio.success()
 
