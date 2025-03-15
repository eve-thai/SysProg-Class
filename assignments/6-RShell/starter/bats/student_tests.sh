#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file


DSH="./dsh"

run_dsh() {
    run ./dsh <<< "$1"
}

# Test: Check if `ls` runs without errors
@test "Example: check ls runs without errors" {
    run $DSH <<EOF
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]  # Check if the exit status is 0 (success)
    [ -n "$output" ]     # Check if there is any output
}

@test "Test help message" {
    run $DSH -h

    # Assertions
    [ "$status" -eq 0 ]  # Check if the exit status is 0
    [[ "$output" == *"Usage: ./dsh"* ]]  # Check if the help message is displayed
}

@test "Test ls command in local mode" {
    run_dsh "ls"

    # Assertions
    [ "$status" -eq 0 ]  # Check if the exit status is 0 (success)
    [ -n "$output" ]     # Check if there is any output
}
@test "Test stop-server command" {
    run $DSH <<EOF
stop-server
EOF

    # Assertions
    [ "$status" -eq 0 ]  # Check if the exit status is 0
}

@test "Test exit command" {
    run $DSH <<EOF
exit
EOF

    # Assertions
    [ "$status" -eq 0 ]  # Check if the exit status is 0
}

@test "Test invalid command (should failed)" {
    run $DSH <<EOF
invalid_command
EOF

    # Assertions
    [ "$status" -ne 0 ]  # Check if the exit status is not 0 (failure)
}
