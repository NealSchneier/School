Student Information
------------------------
Neal Schneier neal89 
Michael Chao mchao8 

How to execute the shell
---------------------------
./esh will run the shell
./esh -p dir will run the shell with the plugins
make will make the file. 
only added file is esh-helpers.c file

Important Notes
---------------------------
System works extremely well. Passes all the basic tests and all of our advanced test
except for piping with more than 2 commands that are not the built in commands that we implemented. 


Description of Base Functionality
-----------------------------------
Jobs - Created our own list. When executing a command the parent will remove the current list element from the
list and add a copy of it to our list of jobs. The jobs commands simple prints out the current contents of the 
jobs list. 
fg - Uses the jobs list to get the correct process and bring it to the foreground. Matches based on the Job ID or the most 
recent process.
bg - Continues the process based on the Job ID number. Finds it in the jobs list.
kill - Matches the Job ID and the finds the item in the jobs list and sends the kill signal. 
stop - Cycles through the jobs list and matches it based job id and then then sends the stop process. 
^C - Kills the current running process. We use a signal handler to catch it in our parent shell. 
^Z - Stops the current runing process by the signal handler and it updates the status of the jobs that has been stopped.

Description of Extended Funcitionality
---------------------------------------
I/O - Allows for programs to take files as output and input. Redirects a process' standard input and output.
Pipes - Allows for file descriptors to overwrite the pipes in a standard process to allow for reading from different processes run.
Exclusive Access - Every time that a command needs the foreground it will give the terminal to that process wait for the 
process to complete execution and then give the terminal back to the shell process. 


List of Plugins Implemented
-------------------------------------
Written by Others
	dirs 
	group 111

	NumberTOLetterGrade
	group 120
	
	RGBToHEX
	group 120
	
	TempConverter
	group 120
	
	Alarm
	group 127
	
	Ascii
	group 127
	
	BinaryConverter
	group 127
	
	quadratic 
	group 127
	
	add
	group 130
	
	mul
	group 130
	
	reverse
	group 130
	
	sub
	group 130
	
	Cowsay
	group 138
	
	htoi
	group 138
	
	itoh
	group 138
	
	quit
	group 138
	
	group143_inchestofeet
	group143
	
	group143_mod
	group143
	
	group143_tosscoin
	group143
	
	fact
	group145
	
	height
	group145
	
	homerSays
	group145
	
	mtok
	group145

	dpipe
	dpipe
	
	cd
	gback
	
	deadline
	gback


	
