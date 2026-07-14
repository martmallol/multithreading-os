# Capture the Flag — a multithreaded scheduler game

A twist on *Capture the Flag* where **every team is a process** and **every player is a thread**. Two teams race across a grid toward the enemy flag; the first to capture it wins. A **Game Master** arbitrates the whole thing with semaphores, barriers and mutexes — so the game stays parallel without race conditions or deadlocks.

The catch: how each team moves is decided by a **scheduling strategy** that mimics a real-world OS scheduler (Round Robin, First-Come First-Served, Shortest Job First, and a proportional-share strategy). Swap the strategy and the race changes.

## Play it in the browser (no build needed)

The fastest way to try it is the web demo — it reproduces the game logic in JavaScript, no compiler required:

```
docs/index.html
```

Just open that file in any browser (or serve the `docs/` folder). Each match plays out like this:

1. You're **randomly assigned a team** (red or blue).
2. The board shows four **flashing corner flags** — click one to **place your flag**. The enemy flag goes to the opposite corner.
3. Pick **your** scheduling strategy (and, for strategies that use it, a quantum). The enemy is given a **different random strategy**, so every match is *your scheduler vs a rival one*.
4. Watch the two teams race. You can pause, step, reset, and adjust speed live.

The scheduler panel shows each team's strategy and live distance-to-flag; the stdout panel traces which threads moved each turn.

To publish it as a live site, enable **GitHub Pages** pointing at the `docs/` folder.

> The browser demo is a self-contained reimplementation of the game logic, extended with per-match variability (see below). The native C++ engine is the original, simpler version driven by `config/`.

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

- Two teams — **red** and **blue** — start near opposite-corner flags. Red always moves first.
- On its turn, a team's players step one square (Up / Down / Left / Right) toward the enemy flag. Players can't leave the map, move onto a wall, or move onto a square another player occupies.
- The first team to reach the enemy flag wins. If neither team can make progress, the game ends in a **draw**.

Strategy and quantum for the native engine are chosen in `main.cpp`; board size, flags and starting positions live in `config/config_parameters.csv`.

### Per-match variability (browser demo)

To keep every match interesting — rather than always favouring whoever moves first — the demo adds several sources of variation on top of the base rules:

- **Independent strategies per team.** You choose your strategy; the enemy gets a *different* random one. The outcome reflects which scheduler suits the map, not just turn order.
- **Randomised starting positions.** Each team scatters its four threads at random inside its home corner (clear of walls and each other), so the two sides are no longer perfectly symmetric.
- **Random walls.** Obstacles (`▓`) are sprinkled across the board, placed **point-symmetrically** through the centre so neither side gets more of them, and always keeping each flag's immediate area clear.
- **Shared quantum.** Both teams use the same quantum value; the slider is enabled whenever *either* team's strategy uses one (Round Robin or Fair Share).

### Movement & draw detection (browser demo)

- **Pathfinding.** Players navigate with a breadth-first shortest-path search that routes **around walls and other players**. If a route to the enemy flag exists, the player takes its first step; if none exists right now, the player waits. This prevents pieces from getting wedged between a wall and a teammate.
- **Deadlock → draw.** If neither team gets any closer to the enemy flag for several rounds, the match is declared a **draw (deadlock)** instead of running forever.

## Strategies

Each strategy mimics a real scheduler algorithm. They all move players toward the enemy flag the same way — what changes is *which* thread advances each round and *how many* steps it gets. (The UI shows friendly names; the C++ `estrategia` enum names are noted in parentheses.)

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
