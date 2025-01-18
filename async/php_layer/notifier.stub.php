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
     * Reserved for internal use:
     * reactor_notifier_notify_t notify_fn
     * reactor_remove_callback_t remove_callback_fn
     */
    private int $reserved = 0;

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

    /**
     * Returns a string representation of the Notifier that can be used for debugging purposes.
     */
    public function __toString(): string {}
}
