<?php

/** @generate-class-entries */

/**
 * @strict-properties
 * @not-serializable
 */
final class Fiber
{
    public function __construct(callable $callback) {}

    public function start(mixed ...$args): mixed {}

    public function resume(mixed $value = null): mixed {}

    public function throw(Throwable $exception): mixed {}

    public function isStarted(): bool {}

    public function isSuspended(): bool {}

    public function isRunning(): bool {}

    public function isTerminated(): bool {}

    public function getReturn(): mixed {}

    public function getContext(): FiberContext {}

    public function addShutdownHandler(callable $callable): void {}

    public function removeShutdownHandler(callable $callable): void {}

    public static function getCurrent(): ?Fiber {}

    public static function suspend(mixed $value = null): mixed {}
}


/**
 * @strict-properties
 * @not-serializable
 */
final class FiberContext
{
    public function get(string $key): mixed {}
    public function has(string $key): bool {}
    public function set(string $key, mixed $value): void {}
    public function del(string $key): void {}

    public function findObject(string $type): object|null {}
    public function bindObject(object $object, string|null $type = null, bool $replace = false): void {}
    public function unbindObject(string $type): void {}
}

final class FiberError extends Error
{
    public function __construct() {}
}
