<?php

/** @generate-class-entries */

namespace Async;

/**
 * An event handler capable of handling the occurrence of an event or an error.
 */
interface CallbackInterface
{
    /**
     * Invoke the callback with the given event and optional error.
     *
     * @param mixed $event The event data.
     * @param \Throwable|null $error Optional error related to the event.
     */
    public function __invoke(mixed $event, ?\Throwable $error = null): void;

    /**
     * Dispose of the callback, releasing any associated resources.
     */
    public function disposeCallback(): void;
}

/**
 * An interface for objects that can be used to notify about the occurrence of an event.
 */
interface NotifierInterface
{
    /**
     * Adds a callback to the event.
     *
     * @param CallbackInterface $callback The callback to add.
     * @return static
     */
    public function addCallback(CallbackInterface $callback): static;

    /**
     * Removes a callback from the event.
     *
     * @param CallbackInterface $callback The callback to remove.
     * @return static
     */
    public function removeCallback(CallbackInterface $callback): static;
}
