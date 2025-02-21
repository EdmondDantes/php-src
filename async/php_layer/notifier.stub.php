<?php

/** @generate-class-entries */

namespace Async;

/**
 * The class that can be used to notify about the occurrence of an event.
 *
 * @strict-properties
 * @not-serializable
 */
class Notifier
{
    private array $callbacks = [];

    /**
     * Equals to TRUE if the Notifier is terminated.
     * If the Notifier is in the Terminated state, it will never generate any events again.
     * Attempting to add such a Notifier to the Resume object will result in an error.
     */
    public readonly bool $isTerminated = false;

    /**
     * The string representation of the Notifier.
     */
    private mixed $toString;

    /**
     * Returns the number of registered callbacks.
     */
    final public function getCallbacks(): array {}

    /**
     * Adds a callback to the event.
     *
     * @param Closure $callback The callback to add.
     * @return static
     */
    public function addCallback(Closure $callback): static {}

    /**
     * Removes a callback from the event.
     *
     * @param Closure $callback The callback to remove.
     * @return static
     */
    public function removeCallback(Closure $callback): static {}

    /**
     * Notifies all registered callbacks about the event.
     */
    final protected function notify(mixed $event, ?\Throwable $error = null): void {}

    /**
     * Close the Notifier.
     */
    final protected function close(): void {}

    /**
     * Returns a string representation of the Notifier that can be used for debugging purposes.
     */
    public function toString(): string {}
}

/**
 * Class for internal use. Implements custom handlers for C code.
 *
 * @strict-properties
 * @not-serializable
 */
final class NotifierEx extends Notifier
{
    private function __construct() {}
}

/**
 * Trigger object for canceling pending operations.
 *
 * @strict-properties
 * @not-serializable
 */
final class Cancellation
{
    public readonly Notifier $notifier;

    public function isCancelled(): bool {}

    public function cancel(\Throwable $error): void {}
}