# Simple-Shell-Interpreter
I made a simple SSI for linux OS

## About

This shell mimics basic functionality of the Bash shell, supporting:

- Execution of external commands (e.g., `ls -l`)
- Foreground and background process management (`bg`, `bglist`)
- Built-in commands: `cd`, `pwd`, `exit`
- Handling of special paths: `~`, `.`, `..`
- Proper prompt formatting: `username@hostname: /path >`


**This program only runs on Linux based operating systems**

To run the program, compile using the makefile: `make` or `make ssi`

Run the shell using ./ssi

To **exit**, type exit into the terminal

## Academic Integrity Notice

This project was developed as coursework for CSC360 (Operating Systems) at the University of Victoria (Summer 2025).

Starter files, such as `inf.c`, `args.c` and `sample.c`, were provided by the UVic course team. All other code is original unless otherwise stated.
