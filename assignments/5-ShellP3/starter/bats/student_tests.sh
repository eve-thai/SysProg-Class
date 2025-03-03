#!/usr/bin/env bats

# Helper function to run dsh with input
run_dsh() {
    run "./dsh" <<EOF
$1
EOF
}

@test "Test: valid command execution" {
    run_dsh "echo 'Hello World'"

    # Assertions
    [ "$status" -eq 0 ]
    [ "$output" = "Hello World" ]
}

@test "Test: empty command input" {
    run_dsh ""

    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" =~ "warning: no commands provided" ]]
}

@test "Test: exit command" {
    run_dsh "exit"

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Test: command with too many pipes" {
    run_dsh "echo hello | echo world | echo again | echo too | echo many | echo pipes"

    # Assertions
    [ "$status" -ne 0 ]
    [[ "$output" =~ "too many pipes" ]]
}

@test "Test: invalid command" {
    run_dsh "nonexistentcommand"

    # Assertions
    [ "$status" -ne 0 ]
    [[ "$output" =~ "command not found" ]]
}
