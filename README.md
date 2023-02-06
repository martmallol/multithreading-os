# Operative Systems Group Project - Capture the Flag!

## 2022, 2nd Semester

## Compilation

Execute the following command:

```
make
```

This will create the game's executable called `juego`. Once you open it, the game will run with the inputs located in the .csv files that belong in the `config/` directory.

If you want to delete every file that was created by the `Makefile` one, execute the following command:

```
make clean
```

You can find a game example for every possible strategy on the following files:
-  `resultadoSECUENCIAL.txt`
- `resultadoRR.txt`
- `resultadoSHORTEST.txt`
- `resultadoUDS.txt`

## Overview
This game's known as *'Capture the Flag'*. It has a major tweak though: every team is a **process** and  every player of it is a **thread**!

There are two teams: 'red' & 'blue'. The 'red' team always starts first. Both teams move on its turn and a player only has 4 movement choices: Up, Down, Left and Right. It a player reaches the limit of the map, it won't be able tu cross out of bounds. Moreover, a player won't be able to reach a map square where another player is actually standing on.

The first team that captures the opponent's flag is the winner. 

The entity that will take charge on the game's direction will be **'The Game Master'**. 

It's possible, that the game could be finished on a draw. That happens when every player of both teams gets stuck and can't move on.

### Users should be able to:
- Select a **strategy** (with a proper *quantum*) for both teams on the `main.cpp` file.
- Compile the game and run it!
- Read how the game unfolded and its final result on the **console**.

### Strategies
The strategies were made with the intention of mimmiching the algorithms from a real-world Scheduler (for example: Round Robin, First-come First-served, Shortest Job First, etc.).

These are the strategies available in the game:
- **Sequential (called: SECUENCIAL):** Without a specific order, it moves every player one step closer to the opponent's flag.
- **Round Robin (called: RR):** The team will move *'quantum'* steps, one per player until the quantum runs out. If the amount of players is greater than the quantum, every player of the team will move 1 step. On the other hand, if the quantum is greater than the amount of players that the team presents, once the last player moves, the round starts over again, until the quantum runs out.
- **Shortest Distance First (called: SHORTEST):** The player closest to the opponent's flag will be the only that will move. It will move only one step at a turn.
- **The programmer's strategy (called: USTEDES):** The strategy's moto is: *"Everyone will have their chance"*. The farthest player to the opponent's flag will be the one assigned with the most steps available. The second farthests will be assigned with the second most steps available and so on... That makes the player with the shortest distance to the opponent's flag being assigned the least amount of steps available for the team. The amount of steps that every has depends on the *quantum* value.

### Built with
C++, processes, threads, semaphores, barriers, and mutexes.

### What we learnt
- How to prevent **race conditions** and **deadlocks** by implementing **semaphores**, **mutexes** and **barriers** on our game. These tools are *very* important while programming a multithread application/game.
- Manipulate processes and threads so we can make them do what we intend to, but *not* making the game sequential as a consecuence. The parallelism was one of the most important things to preserve, and it was indeed preserved.
- The importance, power and efficieny of 'parallelism' over 'sequentialism'.
- How to create a legible & clean Makefile that takes charge of the whole project's compilation.
