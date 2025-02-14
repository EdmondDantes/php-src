<?php

/** @generate-class-entries */

namespace Async;

/**
 * Starts the Scheduler to manage Fibers.
 * The Scheduler can only start in the main execution thread. Attempting to start it from a Fiber results in an error.
 */
function launchScheduler(): void {}

/**
 * Suspends the execution of a Fiber until the Resume object is resolved.
 * If the Resume object is destructed for any reason, it automatically resumes the Fiber.
 */
function wait(?Resume $resume = null): void {}

/**
 * Create a new Fiber that will execute the specified task.
 */
function run(callable $task, mixed ... $args): void {}

/**
 * Creates a coroutine (Fiber) that will execute as soon as an opportunity arises.
 */
function async(callable $task, mixed ... $args): FiberHandle {}

/**
 * Creates a microtask that is guaranteed to execute before events are processed, and other Fibers are started,
 * and immediately after the Scheduler takes control.
 */
function defer(callable $microtask): void {}

/**
 * Creates a callback microtask that will execute as soon
 * as the specified time expires and the Scheduler takes control.
 * Callback allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the Callback object is destroyed by the garbage collector or goes out of scope.
 */
function delay(int $timeout, callable|Callback $callback): void {}

/**
 * Creates a callback microtask that will execute at regular intervals
 * as long as the specified time interval has not expired and the Scheduler takes control.
 * Callback allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the CallbackInterface object is destroyed by the garbage collector or goes out of scope.
 */
function repeat(int $interval, Callback $callback): void {}

/**
 * Registers a callback that will be executed when the specified signal is received.
 * Callback allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the Callback object is destroyed by the garbage collector or goes out of scope.
 */
function onSignal(int $sigNumber, Callback $callback): void {}

/**
 * Execute an external program.
 */
function exec(
    string $command,
    array &$output      = null,
    int &$result_code   = null,
    int $timeout        = 0,
    string $cwd         = null,
    array $env          = null,
    array $options      = null
): string {}
void {}
