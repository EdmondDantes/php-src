<?php

/** @generate-class-entries */

namespace Async;

/**
 * @strict-properties
 * @not-serializable
 */
final class Key
{
    public readonly string $description;
    public function __construct(string $description = '') {}
    public function __debugInfo(): string {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class Context
{
    /**
     * Create a new current context.
     * If the current context already exists, it will be replaced with the new one.
     */
    public static function newCurrent(): Context {}
    /**
     * Get the current context.
     */
    public static function current(bool $createIfNull = false): ?Context {}
    /**
     * Creates a new context, sets the current context as the parent for the new one,
     * and sets the new context as the current one.
     */
    public static function overrideCurrent(bool $weakParent = false): Context {}

    /**
     * Return a current local context.
     * The local context can be considered the execution context of a Fiber.
     */
    public static function local(): Context {}

    public function __construct(?Context $parent = null, bool $weakParent = false) {}

    public function find(string|object $key): mixed {}
    public function get(string|object $key): mixed {}
    public function has(string|object $key): bool {}

    public function findLocal(string|object $key): mixed {}
    public function getLocal(string|object $key): mixed {}
    public function hasLocal(string|object $key): bool {}

    public function setKey(string|object $key, mixed $value, bool $replace = false): Context {}
    public function delKey(string|object $key): Context {}

    public function getParent(): ?Context {}

    public function isEmpty(): bool {}
}

final class ContextException extends \Exception {}