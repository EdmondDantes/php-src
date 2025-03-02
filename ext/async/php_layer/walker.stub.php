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
    /**
     * Iterates over the given iterable asynchronously
     * and calls the given callback function for each element.
     *
     * param iterable      $iterator       The iterable to walk over.
     * param callable      $function       The callback function to call for each element.
     * param mixed         $customData     Custom data to pass to the callback function.
     * param callable|null $defer          The callback function to call when the iteration is finished.
     * param int           $concurrency    The number of concurrent operations.
     *
     * return Walker
     */
    public static function walk(
        iterable $iterator,
        callable $function,
        mixed $customData = null,
        ?callable $defer = null,
        int $concurrency = 0
    ): Walker {}

    /**
     * Iterates over the given iterable asynchronously
     * and calls the given callback function for each element.
     * The callback function should return a value.
     *
     * param iterable      $iterator       The iterable to walk over.
     * param callable      $function       The callback function to call for each element.
     * param mixed         $customData     Custom data to pass to the callback function.
     * param callable|null $defer          The callback function to call when the iteration is finished.
     * param int           $concurrency    The number of concurrent operations.
     *
     * @return Walker
     */
    public static function map(
        iterable $iterator,
        callable $function,
        mixed $customData = null,
        ?callable $defer = null,
        int $concurrency = 0
    ): Future {}

    /**
     * Iterates over the given iterable asynchronously.
     *
     * param    iterable    $iterator       The iterable to iterate over.
     * param    bool        $returnArray    Whether to return the result as an array.
     * param    int         $concurrency    The number of concurrent operations.
     *
     * @return Future
     */
    public static function iterate(
        \Iterator|\IteratorAggregate $iterator,
        bool $returnArray = false,
        int $concurrency = 0
    ): Future {}

    public readonly bool $isFinished = false;
    private iterable $iterator;
    private mixed $customData;
    private mixed $defer;

    public function getFuture(): Future {}

    private function run(): void {}
    private function next(): void {}
    public function cancel(): void {}
}