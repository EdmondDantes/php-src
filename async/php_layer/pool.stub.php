<?php

/** @generate-class-entries */

namespace Async;

/**
 * @template-covariant T
 */
class Pool
{
    protected callable   $factory;
    protected int        $maxPoolSize;
    protected int        $minPoolSize        = 0;
    protected int        $timeout            = 0;
    protected int        $delayPoolReduction = 0;

    /**
     * Pool constructor.
     */
    public function __construct(
        callable    $factory,
        int         $maxPoolSize,
        int         $minPoolSize        = 0,
        int         $timeout            = 0,
        int         $delayPoolReduction = 0,
    ) {}

    /**
     * Borrow an object from the pool.
     * @return T|null
     */
    public function borrow(): object|null {}

    /**
     * Return an object to the pool.
     * @param T $object
     */
    public function return(object $object): void {}

    /**
     * Rebuild pool state and free unused objects.
     *
     */
    public function rebuild(): void {}

    /**
     * Get the maximum pool size.
     *
     */
    public function getMaxPoolSize(): int {}

    /**
     * Get the minimum pool size.
     *
     */
    public function getMinPoolSize(): int {}

    /**
     * Get the timeout for borrowing an object.
     */
    public function getMaxWaitTimeout(): int {}

    /**
     * Get used objects count.
     */
    public function getUsed(): int {}

    /**
     * Get the pool size.
     */
    public function getPoolSize(): int {}
}