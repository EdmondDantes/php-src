<?php

/** @generate-class-entries */

namespace Async;

/**
 * Suspends the execution of a Fiber until the Resume object is resolved.
 * If the Resume object is destructed for any reason, it automatically resumes the Fiber.
 */
function await(Resume $resume): void {}

/**
 * Creates a coroutine (Fiber) that will execute as soon as an opportunity arises.
 */
function async(callable $task): FiberHandle {}

/**
 * Creates a microtask that is guaranteed to execute before events are processed, and other Fibers are started,
 * and immediately after the Scheduler takes control.
 *
 * You can pass an EventCallbackInterface object as an argument, allowing you to cancel the execution of the task.
 */
function defer(callable $microtask): void {}

/**
 * Creates a callback microtask that will execute as soon
 * as the specified time expires and the Scheduler takes control.
 * EventCallbackInterface allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the EventCallbackInterface object is destroyed by the garbage collector or goes out of scope.
 */
function delay(int $timeout, callable $callback): void {}

/**
 * Creates a callback microtask that will execute at regular intervals
 * as long as the specified time interval has not expired and the Scheduler takes control.
 * EventCallbackInterface allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the EventCallbackInterface object is destroyed by the garbage collector or goes out of scope.
 */
function repeat(int $interval, EventCallbackInterface $callback): void {}

/**
 * Registers a callback that will be executed when the specified signal is received.
 * EventCallbackInterface allows you to cancel the callback.
 * The callback will also be immediately canceled
 * if the EventCallbackInterface object is destroyed by the garbage collector or goes out of scope.
 */
function onSignal(int $sigNumber, EventCallbackInterface $callback): void {}
