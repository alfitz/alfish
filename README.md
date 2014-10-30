##alfish

*alfish is a command line shell program that can interpret and execute simple commands.*

###usage
**[command] [options]** --- execute the command<br>
**[command] [options] &** --- execute the command in the background<br>
**[command1] [options] | [command2] [options]** --- pipe the stdout of command1 to stdin of command2<br>
**[command] [options] > [file]** --- redirect stdout to file<br>
**[command] [options] 1> [file]** --- redirect stdout to file<br>
**[command] [options] 2> [file]** --- redirect stderr to file<br>
**[command] [options] &> [file]** --- redirect stdout and stderr to file<br>
**[command] [options] < [file]** --- redirect file contents to stdin of command<br>
**[command] [options] >> [file]** --- append stdout of command to file<br>
**[command] [options] 2>> [file]**--- append stderr of command to file<br>
