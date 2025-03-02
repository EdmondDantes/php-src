<?php

/** @generate-class-entries */

namespace Async;

/**
 * An event handler capable of handling the occurrence of an event or an error.
 *
 * @strict-properties
 * @not-serializable
 */
final class Closure
{
    public function __construct(callable $callback) {}

   /**
     * Dispose of the Closure, releasing any associated resources.
     */
    public function disposeClosure(): void {}
}

