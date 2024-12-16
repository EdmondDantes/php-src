<?php

/** @generate-class-entries */

namespace Async;

/**
 * An event handler capable of handling the occurrence of an event or an error.
 */
class Callback
{
    /**
     * @param callable $callback
     */
    private mixed $callback;

    /**
     * @param Notifier|null $notifier
     */
    private ?Notifier $notifier = null;

    public function __construct(callable $callback) {}

    /**
     * Returns TRUE if the callback is registered with a notifier.
     */
    public function isRegistered(): bool {}    

   /**
     * Dispose of the callback, releasing any associated resources.
     */
    public function disposeCallback(): void {}
}

