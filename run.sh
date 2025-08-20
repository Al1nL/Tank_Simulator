#!/bin/bash

# Paths (adjust if needed)
MAPS_DIR="./maps_squared"
ALGOS_DIR="./Algorithms"
GM_DIR="./GameManager"
OUT_DIR="./logs"
MAIN_EXEC="./simulator_212535058_324022904"

# Check if executable exists
if [[ ! -x "$MAIN_EXEC" ]]; then
    echo "Error: $MAIN_EXEC not found or not executable"
    exit 1
fi

mkdir -p "$OUT_DIR"

# Iterate over all maps
for map in "$MAPS_DIR"/*.txt; do
    map_name=$(basename "$map" .txt)
    echo ">>> Running on map: $map_name"

    # Per-map log folder
    map_out="$OUT_DIR/$map_name"
    mkdir -p "$map_out"

    # Iterate over all algorithm .so files
    for algo1 in "$ALGOS_DIR"/*.so; do
        for algo2 in "$ALGOS_DIR"/*.so; do
            # Skip if the same algorithm
            if [[ "$algo1" == "$algo2" ]]; then
                continue
            fi

            algo1_name=$(basename "$algo1" .so)
            algo2_name=$(basename "$algo2" .so)

            echo "    Match: $algo1_name vs $algo2_name"

            logfile="$map_out/${algo1_name}_vs_${algo2_name}.log"

            # Run and redirect stdout+stderr into logfile
            $MAIN_EXEC -comparative \
                game_map="$map" \
                game_managers_folder="$GM_DIR" \
                algorithm1="$algo1" \
                algorithm2="$algo2" \
                -verbose > "$logfile" 2>&1
        done
    done
done
