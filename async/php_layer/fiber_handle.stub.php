<?php

/** @generate-class-entries */

namespace Async;

/**
 * An interface for representing Fiber as Event Handle.
 */
final class FiberHandle implements EventHandleInterface
{
    public function addCallback(EventCallbackInterface $callback): static {}

    public function isStarted(): bool {};

    public function isSuspended(): bool {};

    public function isRunning(): bool {};

    public function isTerminated(): bool {};

    public function &getContext(): array {};

    /**
     * Allows canceling the execution of a coroutine with an exception.
     */
    public function cancelWith(\Throwable $error): void {};
}
