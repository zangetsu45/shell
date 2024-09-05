# Custom Shell in C++

This project implements a basic POSIX-style shell in C++. The shell supports:

- **Command Execution**: Run system commands with arguments.
- **Built-in Commands**: Includes `cd`, `echo`, `pwd`, `ls`, `history`, `pinfo`, and `search`.
- **I/O Redirection**: Supports input/output redirection (`<`, `>`, `>>`).
- **Pipelines**: Handles piped commands (`command1 | command2`).
- **Background Processes**: Execute commands in the background using `&`.

The shell also handles `CTRL-C` and `CTRL-Z` for managing foreground processes.

to run this file use the following commands
<pre>
        g++ shell.cpp -o shell
        ./shell
</pre>
