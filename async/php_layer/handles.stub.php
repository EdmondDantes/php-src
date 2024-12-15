<?php

/** @generate-class-entries */

namespace Async;

final class TimerHandle implements EventHandleInterface
{
    public function __construct(int $seconds) {}

    public function addCallback(CallbackInterface $callback): static {}
}

/**
 * An interface for representing Fiber as Event Handle.
 */
final class FiberHandle implements EventHandleInterface
{
    public function addCallback(CallbackInterface $callback): static {}

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


final class SignalHandle implements EventHandleInterface
{
    public function __construct(int $sigNumber) {}

    public function addCallback(CallbackInterface $callback): static {}
}

final class ThreadHandle implements EventHandleInterface
{
    public function __construct(int $threadId) {}

    public function addCallback(CallbackInterface $callback): static {}
}


final class ProcessHandle implements EventHandleInterface
{
    public function __construct(int $processId) {}

    public function addCallback(CallbackInterface $callback): static {}
}
