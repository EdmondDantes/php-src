<?php

/** @generate-class-entries */

namespace Async;

interface ChannelStateInterface
{
    public function isClosed(): bool;
    public function isFull(): bool;
    public function isEmpty(): bool;
    public function isNotEmpty(): bool;
    public function isProducingFinished(): bool;
    public function getCapacity(): int;
    public function getUsed(): int;
}

interface ProducerInterface extends ChannelStateInterface
{
    public function send(mixed $data, int $timeout = 0, ?Notifier $cancellation = null, ?bool $waitOnFull = true): void;
    public function trySend(mixed $data): void;
    public function waitUntilWritable(int $timeout = 0, ?Notifier $cancellation = null): bool;
    public function finishProducing(): void;
}

interface ConsumerInterface extends ChannelStateInterface, \Iterator, \Countable
{
    public function receive(int $timeout = 0, ?Notifier $cancellation = null): mixed;
    public function tryReceive(): mixed;
    public function waitUntilReadable(int $timeout = 0, ?Notifier $cancellation = null): bool;
    public function discardData(): void;
    public function finishConsuming(): void;
}

interface ChannelInterface extends ProducerInterface, ConsumerInterface
{
    public function close(): void;
    public function getNotifier(): Notifier;
}

class ChannelException extends \Exception {}
final class ChannelWasClosed extends ChannelException {}
final class ChannelIsFull extends ChannelException {}

final class ChannelNotifier extends Notifier {}

/**
 * @strict-properties
 * @not-serializable
 */
class Channel implements ChannelInterface
{
    public readonly ?\Fiber $owner = null;

    public function __construct(int $capacity = 1, ?\Fiber $owner = null, bool $expandable = false) {}

    public function send(mixed $data, int $timeout = 0, ?Notifier $cancellation = null, ?bool $waitOnFull = true): void {}

    public function trySend(mixed $data): void {}

    public function receive(int $timeout = 0, ?Notifier $cancellation = null): mixed {}

    public function tryReceive(): mixed {}

    public function waitUntilWritable(int $timeout = 0, ?Notifier $cancellation = null): bool;

    public function waitUntilReadable(int $timeout = 0, ?Notifier $cancellation = null): bool;

    public function finishProducing(): void {}

    public function finishConsuming(): void {}

    public function discardData(): void {}

    public function close(): void {}

    public function isClosed(): bool {}

    public function isFull(): bool {}

    public function isEmpty(): bool {}

    public function isNotEmpty(): bool {}

    public function isProducingFinished(): bool {}

    public function getCapacity(): int {}

    public function getUsed(): int {}

    public function getNotifier(): Notifier {}

    public function current(): mixed {}

    public function key(): mixed {}

    public function next(): void {}

    public function rewind(): void {}

    public function valid(): bool {}

    public function count(): int {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ThreadChannel extends Channel
{
    //public function getSenderThreadId(): int {}
    //public function getReceiverThreadId(): int {}
}

/**
 * @strict-properties
 * @not-serializable
 */
final class ProcessChannel extends Channel
{
    //public function getSenderProcessId(): int {}
    //public function getReceiverProcessId(): int {}
}