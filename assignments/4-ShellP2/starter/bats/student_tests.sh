#!/usr/bin/env bats

# File: student_tests.sh
# A test suite for dsh shell functionality

# Helper function to start dsh and send commands
run_dsh() {
    run ./dsh <<< "$1"
}

# --- BASIC EXTERNAL COMMAND TESTS ---
@test "External Command: ls runs without errors" {
    run_dsh "ls"
    [ "$status" -eq 0 ]
}

@test "External Command: echo prints text" {
    run_dsh "echo hello"
    [[ "$output" =~ "hello" ]]
}

@test "External Command: echo preserves spaces inside quotes" {
    run_dsh 'echo "  hello     world  "'
    [[ "$output" =~ "  hello     world  " ]]
}

@test "External Command: pwd prints working directory" {
    run_dsh "pwd"
    [[ "$output" =~ "/" ]]  # Should return a valid path
}

# --- BUILT-IN COMMAND TESTS ---
@test "Built-in Command: cd to valid directory" {
    run ./dsh <<EOF
    mkdir testdir
    cd testdir
    pwd
EOF
    [[ "$output" =~ "testdir" ]]

    run ./dsh <<EOF
    cd ..
    rmdir testdir
EOF
}

@test "Built-in Command: cd to non-existent directory" {
    run_dsh "cd does_not_exist"
    [ "$status" -eq 0 ]  # Should not crash
}

@test "Built-in Command: dragon outputs dragon message" {
    run_dsh "dragon"
    [[ "$output" =~ "You have summoned the dragon" ]]
}

@test "Built-in Command: exit terminates shell" {
    run ./dsh <<< "exit"
    [ "$status" -eq 0 ]
}

# --- QUOTED ARGUMENT HANDLING ---
@test "Quoted Argument: echo handles spaces within quotes" {
    run_dsh 'echo "  hello    world  "'
    [[ "$output" =~ "  hello    world  " ]]
}

@test "Quoted Argument: multiple quoted arguments" {
    run_dsh 'echo "hello" "world"'
    [[ "$output" =~ "hello world" ]]
}


# --- EDGE CASES ---
@test "Edge Case: empty input does nothing" {
    run_dsh ""
    [ "$status" -eq 0 ]
}

@test "Edge Case: excessive spaces before command" {
    run_dsh "         ls"
    [ "$status" -eq 0 ]
}

@test "Edge Case: excessive spaces between command and args" {
    run_dsh "echo        hello"
    [[ "$output" =~ "hello" ]]
}

@test "Edge Case: invalid command returns warning" {
    run_dsh "invalidcommand"
    [[ "$output" =~ "exec failed" ]]
}

@test "Edge Case: too many arguments" {
    long_arg=$(printf 'arg%.0s' {1..1000})  # Create a long argument string
    run_dsh "echo $long_arg"
    [[ "$output" =~ "exec failed" ]]  # Should fail gracefully
}
