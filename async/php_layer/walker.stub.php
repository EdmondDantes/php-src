<?php

/** @generate-class-entries */

namespace Async;

/**
 * The class provides asynchronous iteration of elements that are passed to the callback function.
 * If the callback function is blocked by an I/O operation, the iteration will continue in a new fiber.
 *
 * @strict-properties
 * @not-serializable
 */
final class Walker
{
    public static function walk(
        iterable $iterator,
        callable $function,
        mixed $customData = null,
        ?callable $defer = null,
        int $concurrency = 0
    ): Walker {}

    public readonly bool $isFinished = false;
    private iterable $iterator;
    private mixed $customData;
    private mixed $defer;

    private function run(): void {}
    private function next(): void {}
    public function cancel(): void {}

    public function getFuture(): Future {}
}