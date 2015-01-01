# garry

This file and the associated source code written
by Rob Upcraft at Bethel University.

This is the archived code (I'm no longer actively maintaining it) for the chess program I wrote to complete a senior project during my undergraduate studies.  The program (nicknamed "Garry", after [Garry Kasparov](http://en.wikipedia.org/wiki/Garry_Kasparov)), is a basic implementation of a chess AI's middlegame play.

Note that this is meant to be used as a reference for other students or tinkerers interested in the subject of computer chess.  Because it was an academic project, the code was primarily exploratory; common goals like security and code quality were not necessarily leading priorities and as such, portions of the implementation shouldn't necessarily be mimicked (especially for production use).  That said, its overall simplicity may allow it to serve as a supplement to other resources.  For a good discussion of computer chess, I highly recommend reading [Behind Deep Blue](http://press.princeton.edu/titles/7342.html). 

## Building

This chess program can, in theory, be compiled and run on a beowulf supercomputing cluster supporting MPI (I've only tested it on the Bethel University department's cluster, however).  To build an executable, simply run the Makefile.  Note that the MPI library and C++ compiler are required.

## Compilation Options

In the "Setup Defines" section toward the top of the "chess.c" file, the program's parameters can be tweaked to change its behavior.  These parameters are the following:

**SEARCH_DEPTH**

This is the depth to which the program's minimax function will recurse.  A depth of 0 is the base case (just find the best move out of the current legal moves) with higher integers representing further search.  I used a value of 4 for most of my work on the cluster.

**TYPE_WHITE** and **TYPE_BLACK**

These parameters indicate whether a player is HUMAN or an AI player.  These can be changed to allow a human to play an AI player or two AI players to compete.

**FUNCTION_WHITE** and **FUNCTION_BLACK**

These parameters indicate what type of function (ROB, TURING, or BERLINER) an AI player will use to evaluate the state of a board.

## Invoking

To invoke the chess program, type:

	mpirun -n [NODES] ./chess

where [NODES] is the number of cluster nodes made available to the program.  It should be noted that the program requires at least 2 nodes to be run.

As the program is running, a file named "chesslog.txt" is progressively written, saving the sequence of moves from the beginning of the game.  To restore a game (in the event that a player terminates the program or the program crashes) from this log, type:

	mpirun -n [NODES] ./chess -r

## Usage

While the program is running, a chess board will be printed to the terminal window (with black and white pieces prefaced by '#' and '@' characters, respectively) and the prompt, "ENTER MOVE", will be displayed.  At this prompt, the current player's move can be specified by indicating a starting column/row and ending column/row.  For example, to move a piece from position A2 to A4, one could enter, "a2a4".  If a castle is desired, "CK" or "CQ" can be entered (for castle kingside and queenside, respectively).

To terminate the program before the end of a game, type 'q' at the prompt.
