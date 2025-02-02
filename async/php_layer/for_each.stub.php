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
    public static function start(iterable $iterator, callable $function, ?callable $defer = null): void {}

    public readonly bool $isFinished = false;
    private iterable $iterator;
    private mixed $defer;

    private function run(): void {}
    public function cancel(): void {}
}