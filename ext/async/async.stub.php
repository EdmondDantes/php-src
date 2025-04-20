<?php

/** @generate-class-entries */

namespace Async;

interface Awaitable {}

/**
 * Execute the provided closure in non-cancellable mode.
 */
function protect(\Closure $closure): void {}

function any(iterable $triggers): Awaitable {}

function all(iterable $triggers): Awaitable {}

function anyOf(int $count, iterable $triggers): Awaitable {}

function ignoreErrors(Awaitable $awaitable, callable $handler): Awaitable {}

function captureErrors(Awaitable $awaitable): Awaitable {}

function delay(int $ms): void {}

function timeout(int $ms): Awaitable {}

function currentContext(): Context {}

function coroutineContext(): Context {}

/**
 * Returns the current coroutine.
 */
function currentCoroutine(): Coroutine {}

/**
 * Returns the root Scope.
 */
function rootContext(): Context {}

/**
 * Returns the list of all coroutines
 *
 * @return Coroutine[]
 */
function getCoroutines(): array {}

/**
 * Start the graceful shutdown of the Scheduler.
 */
function gracefulShutdown(?CancellationException $cancellationException = null): void {}

/**
 * Execute an external program.
 * @return Future<array{string, int}>
 */
function exec(
    string $command,
    int $timeout        = 0,
    ?string $cwd        = null,
    ?array $env         = null,
    bool $returnAll     = false
): Future {}