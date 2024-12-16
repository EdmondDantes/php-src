<?php

/** @generate-class-entries */

namespace Async;

abstract class Handle implements EventHandleInterface
{
    public function addCallback(CallbackInterface $callback): static {}
}

/**
 * An interface for representing Fiber as Event Handle.
 */
final class FiberHandle extends Handle
{
    public function isStarted(): bool {}

    public function isSuspended(): bool {}

    public function isRunning(): bool {}

    public function isTerminated(): bool {}

    public function &getContext(): array {}

    /**
     * Allows canceling the execution of a coroutine with an exception.
     */
    public function cancelWith(\Throwable $error): void {}
}

final class InputOutputHandle extends Handle
{
    public function __construct(mixed $handle) {}
}

final class TimerHandle extends Handle
{
    public function __construct(int $microseconds) {}
}

final class SignalHandle extends Handle
{
    public function __construct(int $sigNumber) {}
}

final class ThreadHandle extends Handle
{
    public function __construct(int $threadId) {}
}

final class ProcessHandle extends Handle
{
    public function __construct(int $processId) {}
}

final class FileSystemHandle extends Handle
{
    public function __construct(int $processId) {}
}
