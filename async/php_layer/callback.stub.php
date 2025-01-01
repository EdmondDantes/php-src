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
     * The fiber in which the callback was created.
     * It is used in case an exception occurs in the callback,
     * allowing the exception to be passed to the fiber that
     * created the callback.
     */
    private \Fiber $fiber;

    /**
     * @param Notifier[] $notifiers
     */
    private array $notifiers = [];

    /**
     * Resume object.
     * @param WeakReference<Resume>|null $resume
     */
    private ?WeakReference $resume = null;

    public function __construct(callable $callback) {}

   /**
     * Dispose of the callback, releasing any associated resources.
     */
    public function disposeCallback(): void {}
}

