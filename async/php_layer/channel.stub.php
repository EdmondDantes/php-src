<?php

/** @generate-class-entries */

namespace Async;

interface ProducerInterface
{
    public function send(mixed $data, int $timeout = 0, ?Notifier $cancellation = null, ?bool $waitOnFull = true): void;
    public function sendAsync(mixed $data): void;
    public function finishProducing(): void;
    public function isClosed(): bool;
}

interface ConsumerInterface
{
    public function receive(int $timeout = 0, ?Notifier $cancellation = null): mixed;
    public function receiveAsync(): mixed;
    public function discardData(): void;
    public function finishConsuming(): void;
    public function isProducingFinished(): bool;
    public function isClosed(): bool;
}

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

interface ChannelInterface extends ProducerInterface, ConsumerInterface, ChannelStateInterface
{
    public function close(): void;
    public function getNotifier(): Notifier;
}

class ChannelException extends \Exception {}
final class ChannelWasClosed extends ChannelException {}
final class ChannelIsFull extends ChannelException {}
final class ChannelProducingFinished extends ChannelException {}

final class ChannelNotifier extends Notifier {}

/**
 * @strict-properties
 * @not-serializable
 */
class Channel implements ChannelInterface
{
    public readonly ?\Fiber $owner = null;

    public function __construct(int $capacity = 8, ?\Fiber $owner = null, bool $expandable = false) {}

    public function send(mixed $data, int $timeout = 0, ?Notifier $cancellation = null, ?bool $waitOnFull = true): void {}

    public function sendAsync(mixed $data): void {}

    public function receive(int $timeout = 0, ?Notifier $cancellation = null): mixed {}

    public function receiveAsync(): mixed {}

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