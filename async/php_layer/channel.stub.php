<?php

/** @generate-class-entries */

namespace Async;

class ChannelException extends \Exception {}
class ChannelWasClosed extends ChannelException {}
class ChannelIsFull extends ChannelException {}

/**
 * @strict-properties
 * @not-serializable
 */
class Channel
{
    public const int SEND = 1;
    public const int RECEIVE = 2;
    public const int BIDIRECTIONAL = 3;

    public readonly ?\Fiber $ownerFiber = null;

    public function __construct(int $capacity = 8, int $direction = RECEIVE, bool $expandable = false) {}

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

    public function getDirection(): int {}

    public function transferOwnership(\Fiber $fiber): void {}
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