<?php

/** @generate-class-entries */

namespace Async;

abstract class EvHandle implements Notifier
{
    const READABLE = 1;
    const WRITABLE = 2;
    const DISCONNECT = 4;
    const PRIORITY = 8;

    public function __construct(mixed $handle, int $actions = self::READABLE | self::WRITABLE) {}
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
}

final class SocketHandle extends EvHandle
{
}

final class PipeHandle extends EvHandle
{
}

final class TtyHandle extends EvHandle
{
}

final class TimerHandle extends Notifier
{
    public function __construct(int $microseconds) {}
}

final class SignalHandle extends Notifier
{
    public function __construct(int $sigNumber) {}
}

final class ThreadHandle extends Notifier
{
    public function __construct(int $threadId) {}
}

final class ProcessHandle extends Notifier
{
    public function __construct(int $processId) {}
}

final class FileSystemHandle extends Notifier
{
    const EVENT_RENAME = 1;
    const EVENT_CHANGE = 2;

    const FLAG_NONE = 0;
    const WATCH_ENTRY = 1;
    const WATCH_RECURSIVE = 4;

    public function __construct(string $filename, int $flags) {}
}
