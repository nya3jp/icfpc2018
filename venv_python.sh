#!/bin/bash

set -e

root_dir="$(dirname "$0")"
venv_dir="$root_dir/.venv"
reqs_file="$root_dir/requirements.txt"
stamp_file="$venv_dir/.stamp"

if [[ ! -d "$venv_dir" ]]; then
    echo "Creating new virtualenv at $venv_dir ..." >&2
    python3 -m venv "$venv_dir"
fi

if [[ ! -f "$stamp_file" || "$reqs_file" -nt "$stamp_file" ]]; then
    echo "Updating dependencies..." >&2
    "$venv_dir/bin/pip" install -r "$reqs_file"
    touch "$stamp_file"
fi

exec "$venv_dir/bin/python" "$@"
