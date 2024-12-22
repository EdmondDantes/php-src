<?php

/** @generate-class-entries */

namespace Async;

/**
 * A class that links an array of events to the resumption of a `Fiber`.
 *
 * It associates the moment an event occurs with the logic by which the `Fiber` will be resumed.
 * This is a key element for transferring control between `Fibers`.
 *
 * A `Resume` object cannot be created outside the current `Fiber`.
 * The `Resume` object can transition to a resolved state only once,
 * after which it must be destroyed.
 */
final class Resume
{
    /**
     * The `Fiber` that will be resumed.
     */
    private ?\Fiber $fiber = null;

    /**
     * The `Fiber` that will be resumed.
     */
    private bool $isResolved = false;

    /**
     * The value that will be passed to the `Fiber` when it is resumed.
     */
    private mixed $value = null;

    /**
     * The error that will be thrown into the `Fiber` when it is resumed.
     */
    private ?\Throwable $error = null;

    /**
     * Internal Callback object for Notifiers.
     */
    private ?Callback $callback = null;

    private function __construct() {}

    /**
     * Resumes the fiber with a value.
     */
    public function resume(mixed $value = null): void {}

    /**
     * Throws an exception into the fiber.
     */
    public function throw(?\Throwable $error = null): void {}

    public function isResolved(): bool {}

    public function getNotifiers(): array {}

    public function when(Notifier $notifier, ?callable $callback = null): static {}
}
