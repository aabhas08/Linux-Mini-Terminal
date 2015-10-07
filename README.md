# Linux-Mini-Terminal
Implemented a shell in C for Linux using the concepts of semaphores, threads, piping and IO-redirection.


The Assignment details are below

 The goal of the project is to create a user defined interactive shell program that can create and manage new processes. The shell should be able to create a process out of a system program like emacs, vi or any user-defined executable

The following are the specifications for the project. For each of the requirement an appropriate example is given along with it.
 
Specification 1: Display requirement

When you execute your code a shell prompt of the following form must appear:
<username@system_name:curr_dir>
E.g., <Peytr@ICS231:~>

The directory from which the shell is invoked will be the home directory of the shell and should be indicated by "~"
If the user executes "cd" change dir then the corresponding change must be reflected in the shell as well.
E.g., ./a.out
<Peytr@ICS231:~>cd newdir
<Peytr@ICS231:~/newdir>
 

Specification 2: System commands with and without arguments

All other commands are treated as system commands like : emacs, vi and so on. The shell must be able to execute them either in the background or in the foreground.

Foreground processes: For example, executing a "vi" command in the foreground implies that your shell will wait for this process to complete and regain control when this process exits.
Background processes: Any command invoked with "&" is treated as background command. This implies that your shell will spawn that process and doesn't wait for the process to exit. It will keep taking user commands. If the background process exits then the shell must display the appropriate message to the user.

E.g
<Peytr@ICS231:~> ls &
This command  when finished, should print its result to stdout.

<Peytr@ICS231:~>emacs &
<Peytr@ICS231:~> ls -l -a

.
.
.  Execute other commands
.
.

<Peytr@ICS231:~> echo hello

After emacs exits, your shell program should check the exit status of emacs and print it on stderr 
<Peytr@ICS231:~>
emacs with pid 456 exited normally
<Peytr@ICS231:~>

Specification 3: User-defined commands

The following commands must be supported by the shell

-pinfo : prints the process related info of your shell program.
<Peytr@ICS231:~>pinfo
pid -- 231
Process Status -- {R/S/S+/Z}
memory -- 67854 {Virtual Memory}
Executable Path --  ~/a.out

-pinfo <pid> : prints the process info about given pid.
<Peytr@ICS231:~>pinfo 1

-jobs : prints a list of all currently running jobs along with their pid, particularly background jobs, in order of their creation.
<Peytr@ICS231:~>jobs
  [1] emacs Assign.txt [231]
  [2] firefox [234]
  [3] vim [5678]

Here [3] i.e vim is most recent background job, and the oldest one is emacs. 

-kjob <jobNumber> <signalNumber>: takes the job id of a running job and sends a signal value to that process
<Peytr@ICS231:~> kjob 2 9
it sends sigkill to the process firefox, and as a result it is terminated. Here 9 represents the signal number, which is sigkill. For further info, 
lookup man 7 signal 

-fg <jobNumber>: brings background job with given job number to foreground. 
<Peytr@ICS231:~> fg 3
Either brings the 3rd job which is vim to foreground or returns error if no such background number exists.

-overkill:  kills all background process at once

-quit : exits the shell. Your shell should exit only if the user types this "quit" command. It should ignore any other signal from user like : CTRL-D, CTRL-C, SIGINT, SIGCHLD etc.

-CTRL-Z : It should change the status of currently running job to stop, and push it in background process
