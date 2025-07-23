#!/bin/bash

# Root directory (defaults to current if not provided)
ROOT_DIR="${1:-.}"

# Step 1: Replace 'producer_consumer' with 'producer_consumer' in file contents
find "$ROOT_DIR" -type f -print0 | while IFS= read -r -d '' file; do
    # Only process text files
    if grep -Iq . "$file"; then
        sed -i 's/\btransmitter\b/producer_consumer/g' "$file"
    fi
done

# Step 2: Rename files and directories that have 'producer_consumer' in the name
# Process directories *after* files to avoid path issues
find "$ROOT_DIR" -depth -name '*producer_consumer*' -print0 | while IFS= read -r -d '' path; do
    new_path=$(echo "$path" | sed 's/producer_consumer/producer_consumer/g')
    mv "$path" "$new_path"
done

