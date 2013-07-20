import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

def force_shell_termination(shell_process): c.close(force=True)

definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'): logfile = def_module.logfile

c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile, args=['-p', 'plugins/'])
atexit.register(force_shell_termination, shell_process=c)

c.timeout = 2

assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter 27 C K")
assert c.expect("300 K") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter 300 K C")
assert c.expect("27 C") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter -40 C F")
assert c.expect("-40 F") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter -40 F C")
assert c.expect("-40 C") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter 300 F K")
assert c.expect("421 K") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("tempConverter 20 K F")
assert c.expect("-423 F") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("exit");
assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"

shellio.success()
