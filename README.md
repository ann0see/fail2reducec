# Fail2ReduceC

A program to buffer strings and later on send them in bulk via E-Mail or any command which can read from stdin. Developed to get rid of multiple fail2ban startup messages. This is heavily tailored to my needs, but it might still be useful for someone else.

## Problem this wants to solve

Enabling multiple jails in fail2ban and setting the actionstart/actionstop config options to send e-mails if a jail starts or stops, blasts out multiple e-mails. This happens and is very annoying if e.g. fail2ban is restarted. The idea is probably comparable to [mail-buffered](https://github.com/fail2ban/fail2ban/blob/master/config/action.d/mail-buffered.conf) from the standard configuration - however, this doesn't count lines, but is rather based on time. See below for details.

## Concept

This program tries to infer that fail2ban is "finished" with its burst, if we didn't see an output after some period of time (currently 11 seconds). After this period of time, the first invocation of the C program will make the bash script piping the content of a file where fail2bans output is buffered to a command you can specify.
Probably the implementation with locking is overkill.

## How to use

* Compile main.c with some compiler e.g. gcc: `gcc main.c -o fail2reducec`
* In the fail2reduce.sh script change the path to fail2reducec to the path where you put the binary you just compiled
* In your fail2ban configuration set the actionstart/actionstop set the path to fail2reduce.sh. You must call the fail2reduce.sh script with two parameters: the first parameter is the string you want to log, while the second one is the command you want to pipe the buffered content to. The first one is probably the name of the jails which started, while the second one is probably a command to send e-mail. Moreover, you should ensure that the bash script runs in a separate thread (meaning it still runs if fail2ban is finished)
