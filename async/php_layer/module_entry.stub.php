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
 * Suspends the execution of the current Fiber until $awaitable completes its work.
 * The result of $awaitable is returned as the function's return value.
 *
 * If an unhandled exception occurs during the
 * execution of $awaitable, it will be thrown at the call site of this function.
 */
function await(callable|FiberHandle|Future|\Fiber $awaitable, mixed ... $args): mixed {}

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
    array|string|null &$output = null,
    ?int &$result_code  = null,
    int $timeout        = 0,
    ?string $cwd        = null,
    ?array $env         = null,
    ?array $options     = null
): void {}

/**
 * Returns a list of all fibers registered in the Scheduler.
 *
 * @return \Fiber[]
 */
function getFibers(): array {}

/**
 * Returns a list of all Resume objects that have currently suspended Fibers.
 *
 * @return Resume[]
 */
function getResumes(): array {}

/**
 * Start the graceful shutdown of the Scheduler.
 */
function gracefulShutdown(\Throwable|null $throwable = null): void {}