<?php

/** @generate-class-entries */

namespace Async;

abstract class EvHandle extends Notifier
{
    public const int READABLE = 1;
    public const int WRITABLE = 2;
    public const int DISCONNECT = 4;
    public const int PRIORITY = 8;

    public readonly int $triggeredEvents = 0;

    final public function __construct(mixed $handle, int $actions = self::READABLE | self::WRITABLE) {}
}

/**
 * An interface for representing Fiber as Event Handle.
 */
final class FiberHandle extends Notifier
{
    public function __construct() {}

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
    public function __construct() {}
}

final class ProcessHandle extends Notifier
{
    public function __construct() {}
}

final class FileSystemHandle extends Notifier
{
    public const int EVENT_RENAME = 1;
    public const int EVENT_CHANGE = 2;

    public const int FLAG_NONE = 0;
    public const int WATCH_ENTRY = 1;
    public const int WATCH_RECURSIVE = 4;

    public readonly int $triggeredEvents = 0;

    public function __construct(string $filename, int $flags) {}
}
