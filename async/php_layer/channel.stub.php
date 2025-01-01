<?php

/** @generate-class-entries */

namespace Async;

class ChannelException extends \Exception {}
class ChannelWasClosed extends \Exception {}
class ChannelIsFull extends \Exception {}

/**
 * @strict-properties
 * @not-serializable
 */
class Channel
{
    public function __construct(int $capacity = 8, bool $expandable = false) {}

    public function send(mixed $data, int $timeout = 0, ?Notifier $cancellation = null, ?bool $waitOnFull = true):
    void {}

    public function sendAsync(mixed $data): void {}

    public function receive(int $timeout = 0, ?Notifier $cancellation = null): mixed {}

    public function receiveAsync(): mixed {}

    public function close(): void {}

    public function isClosed(): bool {}

    public function isFull(): bool {}

    public function isEmpty(): bool {}

    public function isNotEmpty(): bool {}

    public function getCapacity(): int {}

    public function getUsed(): int {}
}

final class ThreadChannel extends Channel
{
    public function getSenderThreadId(): int {}
    public function getReceiverThreadId(): int {}
}

final class ProcessChannel extends Channel
{
    public function getSenderProcessId(): int {}
    public function getReceiverProcessId(): int {}
}