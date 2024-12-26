<?php

/** @generate-class-entries */

namespace Async;

abstract class EvHandle implements Notifier
{
}

/**
 * An interface for representing Fiber as Event Handle.
 */
final class FiberHandle extends EvHandle
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

final class FileHandle extends EvHandle
{
    public function __construct(mixed $handle) {}
}

final class SocketHandle extends EvHandle
{
    public function __construct(mixed $handle) {}
}

final class PipeHandle extends EvHandle
{
    public function __construct(mixed $handle) {}
}

final class TtyHandle extends EvHandle
{
    public function __construct(mixed $handle) {}
}

final class TimerHandle extends EvHandle
{
    public function __construct(int $microseconds) {}
}

final class SignalHandle extends EvHandle
{
    public function __construct(int $sigNumber) {}
}

final class ThreadHandle extends EvHandle
{
    public function __construct(int $threadId) {}
}

final class ProcessHandle extends EvHandle
{
    public function __construct(int $processId) {}
}

final class FileSystemHandle extends EvHandle
{
    public function __construct(int $processId) {}
}
