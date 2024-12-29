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

    /**
     * Determines if the `Resume` object has been resolved.
     */
    public function isResolved(): bool {}

    /**
     * Returns the Notifiers associated with the `Resume` object.
     */
    public function getNotifiers(): array {}

    /**
     * Returns the Notifiers that have been triggered.
     */
    public function getTriggeredNotifiers(): array {}

    /**
     * Add a Notifier to the `Resume` object with a callback.
     */
    public function when(Notifier $notifier, ?callable $callback = null): static {}
}
