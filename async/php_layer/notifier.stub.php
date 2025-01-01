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
     * Adds a callback to the event.
     *
     * @param Callback $callback The callback to add.
     * @return static
     */
    public function addCallback(Callback $callback): static {}

    /**
     * Removes a callback from the event.
     *
     * @param Callback $callback The callback to remove.
     * @return static
     */
    public function removeCallback(Callback $callback): static {}

    /**
     * Notifies all registered callbacks about the event.
     */
    final protected function notify(mixed $event, ?\Throwable $error = null): void {}
}
