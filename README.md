# Tank Simulator — Assignment 3  

## Contributors  
- Lior Pernik — 324022904  
- Alin Loshevsky — 212535058  

---

## Project Description  
This project implements a **Tank Simulator** capable of running multiple autonomous tank battles concurrently.  
It extends the previous assignments by introducing:  

- **Dynamic loading** of `GameManager` and `Algorithm` shared libraries (`.so`).  
- **Multithreading**, allowing several games to run in parallel.  
- **Two modes of operation**:  
  - **Comparative** — compare multiple GameManagers on a single map with two given algorithms.  
  - **Competition** — run a full tournament between multiple algorithms across multiple maps.  

The simulator is modular: each part (Simulator, GameManager, Algorithm) should work independently and interoperate with other teams’ implementations.  

---

## Build Instructions  
1. From the project root, run:  
   ```bash
   make
   ```
   This builds all 3 parts (Simulator, GameManager, Algorithm).  
2. Alternatively, each folder (`Simulator/`, `GameManager/`, `Algorithm/`) has its own `Makefile` to build individually.  
3. Output:  
   - `./simulator_212535058_324022904` (Simulator executable)  
   - `GameManager_212535058_324022904.so` (shared library - inside GamaManager folder)  
   - `Algorithm_212535058_324022904.so` (shared library - inside Algorithm folder)  

---

## Running the Simulator  

- The simulator produces two types of outputs:  

  1. **Overall results file** (for comparative/competition mode):  
     - Located in `./<mode>_results_<timestamp>.txt`.

  2. **Our GameManager results** (only when the `-verbose` flag is provided):  
     - Stored under `GM_212535058_324022904/`.  
     - For each match, a subfolder is created in the format:  
       ```
       GM_212535058_324022904/alg1_vs_alg2/
       ```
     - Inside each match folder, there is another subfolder for each map:  
       ```
       GM_212535058_324022904/alg1_vs_alg2/map_name/
       ```
     - Each map folder contains:  
       1. `output_alg1_vs_alg2_map_name.txt` — detailed game log.  
       2. `gamesteps_output_alg1_vs_alg2_map_name.txt` — step-by-step visualization of the game flow.  

---

## Map Format  
Maps can contain any of these symbols throughout the game:  
- `#` — Wall  
- `@` — Mine  
- `1` — Tank (Player 1)  
- `2` — Tank (Player 2)  
- `*` - Shell
- (space) — Empty  

---

## Notes
- **GameManager**  
  - Turn order in each round is as follows:  
    1. Shells move.  
    2. Collisions are resolved.
    3. Tanks perform their moves.  
