<?php

/** @generate-class-entries */

namespace Async;

/**
 * @strict-properties
 * @not-serializable
 */
abstract class PollHandle extends Notifier
{
    public const int READABLE = 1;
    public const int WRITABLE = 2;
    public const int DISCONNECT = 4;
    public const int PRIORITY = 8;

    public readonly int $triggeredEvents = 0;

    final private function __construct() {}

    /**
     * Return TRUE if the handle is listening for events in the reactor.
     */
    final public function isListening(): bool {}

    /**
     * Stop listening for events on the handle.
     */
    final public function stop(): void {}
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

    public function fiberContext(): ?Context {}

    public function cancel(): void {}

    /**
     * Allows canceling the execution of a coroutine with an exception.
     */
    public function cancelWith(\Throwable $error): void {}

    /**
     * Define a callback to be executed when the fiber is terminated.
     */
    public function defer(callable $callback): void {}

    /**
     * Remove a previously defined defer handler.
     */
    public function removeDeferHandler(callable $callable): void {}

    /**
     * The method will return a Future object that will return the result of the Fiber execution.
     */
    public function getFuture(): Future {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class FileHandle extends PollHandle
{
    public static function fromResource(mixed $fd, int $actions = self::READABLE | self::WRITABLE): FileHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class SocketHandle extends PollHandle
{
    public static function fromResource(mixed $resource, int $actions = self::READABLE | self::WRITABLE): SocketHandle {}
    public static function fromSocket(mixed $socket, int $actions = self::READABLE | self::WRITABLE): SocketHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class PipeHandle extends PollHandle
{
    //public static function open(string $path, string $mode): PipeHandle {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class TtyHandle extends PollHandle
{
    //public static function open(string $path, string $mode): TtyHandle {}
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

    public function isListening(): bool {}

    public function stop(): void {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class SignalHandle extends Notifier
{
    public readonly int $sigNumber = 0;

    public static function new(int $sigNumber): SignalHandle {}

    public function isListening(): bool {}

    public function stop(): void {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ThreadHandle extends Notifier
{
    public readonly int|null $tid = 0;

    private function __construct() {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ProcessHandle extends Notifier
{
    public readonly int|null $pid = null;
    public readonly int|null $exitCode = null;

    private function __construct() {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class DnsInfoHandle extends Notifier
{
    public static function resolveHost(string $host): DnsInfoHandle {}
    public static function resolveAddress(string $address): DnsInfoHandle {}

    public readonly string|null $host = null;
    public readonly string|null $address = null;

    private function __construct() {}

    public function getInfo(): string {}
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

    public function isListening(): bool {}

    public function stop(): void {}
}
