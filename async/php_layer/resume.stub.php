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
 * @strict-properties
 * @not-serializable
 */
final class Resume
{
    public readonly mixed $result = null;

    /**
     * Predefined callback-behavior for the `Resume` object
     * when the event is triggered, fiber resumes execution.
     * If an error occurs, an exception is thrown.
     */
    const int RESOLVE   = 1;
    /**
     * Predefined callback-behavior for the `Resume` object
     * when the event is triggered, fiber resumes execution with a cancellation exception.
     * If an error occurs, an exception is thrown.
     */
    const int CANCEL    = 2;
    /**
     * Predefined callback-behavior for the `Resume` object
     * when the event is triggered, fiber resumes execution with a timeout exception.
     * If an error occurs, an exception is thrown.
     * This callback can be used only with the `TimerHandle` object.
     */
    const int TIMEOUT   = 3;

    /**
     * Resumes the fiber with a value.
     */
    public function resume(mixed $value = null): void {}

    /**
     * Throws an exception into the fiber.
     */
    public function throw(?\Throwable $error = null): void {}

    /**
     * Determines if the `Resume` object is pending.
     */
    public function isPending(): bool {}

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
     *
     * Special values RESOLVE, CANCEL, and TIMEOUT indicate predefined behavior:
     * * RESOLVE – on success, the fiber continues execution; on error – an exception is thrown.
     * * CANCEL – on success, the fiber resumes with a cancellation exception `CancellationException`.
     * * TIMEOUT – on success, the fiber resumes execution with a timeout exception `TimeoutException`.
     */
    public function when(Notifier $notifier, callable|int $callback = Resume::RESOLVE): static {}
}
