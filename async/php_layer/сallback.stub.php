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

    public function __construct(callable $callback) {}

   /**
     * Dispose of the callback, releasing any associated resources.
     */
    public function disposeCallback(): void {}
}

