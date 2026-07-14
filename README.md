# Capture the Flag — a multithreaded scheduler game

A twist on *Capture the Flag* where **every team is a process** and **every player is a thread**. Two teams race across a grid toward the enemy flag; the first to capture it wins. A **Game Master** arbitrates the whole thing with semaphores, barriers and mutexes — so the game stays parallel without race conditions or deadlocks.

The catch: how each team moves is decided by a **scheduling strategy** that mimics a real-world OS scheduler (Round Robin, First-Come First-Served, Shortest Job First, and a proportional-share strategy). Swap the strategy and the race changes.

## Play it in the browser (no build needed)

The fastest way to try it is the web demo — it reproduces the game logic in JavaScript, no compiler required:

```
docs/index.html
```

Just open that file in any browser (or serve the `docs/` folder). On start you're **randomly assigned a team** and asked to **place your flag in one of the four corners** — the enemy takes the opposite corner. Then pick a scheduling strategy (and quantum) and watch the two teams race. You can pause, step, reset, and adjust speed live.

To publish it as a live site, enable **GitHub Pages** pointing at the `docs/` folder.

## Build & run the native engine (C++)

The real engine is native C++ with pthreads, `semaphore.h` and barriers. On Linux / WSL / macOS with `g++`:

```
make            # builds the ./game executable
./game          # runs with the inputs in config/
make clean      # removes build artifacts
```

`make` produces an executable called `game`, which reads its inputs from the `.csv` files in the `config/` directory and prints how the game unfolded to the console.

You can find a sample run for every strategy in these files:
- `resultadoSECUENCIAL.txt`
- `resultadoRR.txt`
- `resultadoSHORTEST.txt`
- `resultadoUDS.txt`

## How the game works

- Two teams — **red** and **blue** — start in opposite corners. Red always moves first.
- On its turn, a team's players step one square (Up / Down / Left / Right) toward the enemy flag. Players can't leave the map or move onto a square another player occupies.
- The first team to reach the enemy flag wins. If every player of both teams gets stuck, the game ends in a **draw**.

Strategy and quantum for the native engine are chosen in `main.cpp`; board size, flags and starting positions live in `config/config_parameters.csv`.

## Strategies

Each strategy mimics a real scheduler algorithm. They all share the same greedy movement toward the enemy flag — what changes is *which* thread advances each round and *how many* steps it gets. (The UI shows friendly names; the C++ `estrategia` enum names are noted in parentheses.)

- **Sequential** *(enum `SECUENCIAL`, ≈ First-Come First-Served):* No fixed order — every player advances one step toward the flag. No quantum.
- **Round Robin** *(enum `RR`):* The team spends *quantum* steps, one per player in circular order, until it runs out. If there are more players than quantum, each player moves once; if quantum exceeds the player count, the round wraps around until the quantum is spent.
- **Shortest** *(enum `SHORTEST`, ≈ Shortest Job First):* Only the player closest to the enemy flag moves, one step per turn.
- **Fair Share** *(enum `USTEDES`, ≈ proportional-share scheduler):* "Everyone gets their chance" — the farthest player from the flag is assigned the most steps, the closest the least, with the share scaled by the *quantum*.

## Built with

C++, processes, threads, semaphores, barriers and mutexes — plus a self-contained HTML/CSS/JS front end for the browser demo.

## Notes on the concurrency design

- Semaphores, mutexes and barriers coordinate the threads to avoid **race conditions** and **deadlocks**.
- Processes and threads are driven deliberately while preserving real parallelism — the game is never silently serialized.
- The `Makefile` handles the whole build.
