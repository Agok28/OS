#!/bin/bash

perform_missing_permissions_verification() {
    local file="$1"
    local quarantine_dir="$2"

    if [ ! -r "$file" ] || [ ! -w "$file" ] || [ ! -x "$file" ]; then
        ./verify_for_malicious.sh "$file"
        filename=$(basename "$file")
        mv "$file" "$quarantine_dir/$filename"
    fi
}

perform_syntactic_analysis() {
    local file="$1"

    local num_lines=$(wc -l < "$file")
    local num_words=$(wc -w < "$file")
    local num_chars=$(wc -c < "$file")

    if [ "$num_lines" -lt 3 ] && [ "$num_words" -gt 1000 ] && [ "$num_chars" -gt 2000 ]; then
        return 0  # File is corrupted
    fi

    if grep -qE 'corrupted|dangerous|risk|attack|malware|malicious' "$file"; then
        return 0  # File is corrupted
    fi

    return 1  # File is safe
}

isolate_dangerous_files() {
    local file="$1"
    local quarantine_dir="$2"

    filename=$(basename "$file")
    mv "$file" "$quarantine_dir/$filename"
}

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <directory1> <directory2> ... <quarantine_directory>"
    exit 1
fi

quarantine_dir="${@: -1}"
if [ ! -d "$quarantine_dir" ]; then
    echo "Quarantine directory $quarantine_dir does not exist."
    exit 1
fi

for directory in "${@:1:$#-1}"; do
    if [ ! -d "$directory" ]; then
        echo "Directory $directory does not exist."
        continue
    fi

    while IFS= read -r -d '' file; do
        perform_missing_permissions_verification "$file" "$quarantine_dir"
        if perform_syntactic_analysis "$file"; then
            echo "$file"  
            isolate_dangerous_files "$file" "$quarantine_dir"
        else
            echo "SAFE"  
        fi
    done < <(find "$directory" -type f -print0)
done

echo "Script execution completed."
