#!/bin/bash

set -e

cd "$(dirname "$0")"

./venv_python.sh -m pylint examples/
