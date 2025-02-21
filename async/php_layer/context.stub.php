<?php

/** @generate-class-entries */

namespace Async;

/**
 * @strict-properties
 * @not-serializable
 */
final class Context
{
    public function __construct(?Context $parent = null, bool $weakParent = false) {}

    public function find(string|object $key): mixed {}
    public function get(string|object $key): mixed {}
    public function has(string|object $key): bool {}

    public function findLocal(string|object $key): mixed {}
    public function getLocal(string|object $key): mixed {}
    public function hasLocal(string|object $key): bool {}

    public function withKey(string|object $key, mixed $value): Context {}
    public function withoutKey(string|object $key): Context {}
    public function getParent(): ?Context {}

    public function isEmpty(): bool {}
}
