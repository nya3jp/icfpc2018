#!/bin/bash

set -e

root_dir="$(dirname "$0")"

"$root_dir/venv_python.sh" -m pylint examples/
