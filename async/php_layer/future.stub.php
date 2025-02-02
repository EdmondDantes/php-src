<?php

/** @generate-class-entries */

namespace Async;

/**
 * The
 *
 * @strict-properties
 * @not-serializable
 * @template T
 */
final class FutureState extends Notifier
{
    private bool $complete = false;
    private bool $handled = false;

    /**
     * @var T|null
     */
    private mixed $result = null;

    private ?\Throwable $throwable = null;

    public function __construct()
    {
    }

    /**
     * Completes the operation with a result value.
     *
     * @param T $result Result of the operation.
     */
    public function complete(mixed $result): void
    {
    }

    /**
     * Marks the operation as failed.
     *
     * @param \Throwable $throwable Throwable to indicate the error.
     */
    public function error(\Throwable $throwable): void
    {
    }

    /**
     * @return bool True if the operation has completed.
     */
    public function isComplete(): bool
    {
    }

    /**
     * Suppress the exception thrown to the loop error handler if and operation error is not handled by a callback.
     */
    public function ignore(): void
    {
    }
}