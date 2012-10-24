#!/bin/bash

source 'prepare.bash'

# Configuration
start_idx="1"
end_idx="1,000,000"
num_cpus="4"

seq "${start_idx//,/}" "${end_idx//,/}" | parallel -j"$num_cpus" perl summarize.pl {}
