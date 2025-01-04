<?php

/** @generate-class-entries */

namespace Async;

/**
 * @strict-properties
 * @not-serializable
 */
abstract class EvHandle extends Notifier
{
    public const int READABLE = 1;
    public const int WRITABLE = 2;
    public const int DISCONNECT = 4;
    public const int PRIORITY = 8;

    public readonly int $triggeredEvents = 0;

    final private function __construct(mixed $handle, int $actions = self::READABLE | self::WRITABLE) {}
}

/**
 * An interface for representing Fiber as Event Handle.
 *
 * @strict-properties
 * @not-serializable
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

/**
 * @strict-properties
 * @not-serializable
 */
final class FileHandle extends EvHandle
{
    public static function fromResource(mixed $fd): FileHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class SocketHandle extends EvHandle
{
    public static function fromStream(mixed $stream): FileHandle {}
    public static function fromSocket(mixed $socket): FileHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class PipeHandle extends EvHandle
{
    public static function open(string $path, string $mode): PipeHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class TtyHandle extends EvHandle
{
    public static function open(string $path, string $mode): TtyHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class TimerHandle extends Notifier
{
    public readonly int $microseconds = 0;

    public readonly bool $isPeriodic = false;

    public static function newTimeout(int $microseconds): TimerHandle {}

    public static function newInterval(int $microseconds): TimerHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class SignalHandle extends Notifier
{
    public readonly int $sigNumber = 0;

    public static function new(int $sigNumber): SignalHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ThreadHandle extends Notifier
{
    private function __construct() {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ProcessHandle extends Notifier
{
    public static function fromResource(mixed $process): ProcessHandle {}
    private function __construct() {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class FileSystemHandle extends Notifier
{
    public const int EVENT_RENAME = 1;
    public const int EVENT_CHANGE = 2;

    public const int FLAG_NONE = 0;
    public const int WATCH_ENTRY = 1;
    public const int WATCH_RECURSIVE = 4;

    public readonly int $triggeredEvents = 0;

    public readonly string $path = '';

    public readonly int $flags = 0;

    public static function fromPath(string $path, int $flags): FileSystemHandle {}
    private function __construct() {}
}
