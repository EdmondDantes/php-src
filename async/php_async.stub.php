<?php

/** @generate-class-entries */

namespace Async;

interface FutureInterface
{
    public function getStatus(): int;

    public function isReady(): bool;

    public function isFailed(): bool;

    public function getResult(): mixed;

    public function getError(): ?\Throwable;
}

interface CancellationInterface
{
    public function cancel(): void;
}

interface EventDescriptorInterface extends CancellationInterface
{
    /**
     * Called when the event was registered in the event loop.
     */
    public function onRegistered(callable $disposeCallback): void;
}

interface AwaitableInterface
{
    /**
     * Returns the DeferredResume associated with the awaitable object.
     *
     * @return DeferredResume
     */
    public function getDeferredResume(): DeferredResume;

    /**
     * Returns events associated with the awaitable object.
     *
     * @return EventDescriptorInterface[]
     */
    public function getWaitingEvents(): array;
}

interface CompletionPublisherInterface
{
    /**
     * Registers a subscriber for success events.
     *
     * @param callable $onFulfilled The success callback.
     * @return void
     */
    public function onSuccess(callable $onFulfilled): void;

    /**
     * Registers a subscriber for error events.
     *
     * @param callable $onRejected The error callback.
     * @return void
     */
    public function onError(callable $onRejected): void;

    /**
     * Registers a subscriber for finalization events.
     *
     * @param callable $onFinally The finalization callback.
     * @return void
     */
    public function onFinally(callable $onFinally): void;
}

interface ThenInterface
{
    /**
     * Indicates that the specified objects (events or Deferred)
     * should be ignored if this event has occurred.
     */
    public function thenIgnore(CancellationInterface ...$objects): static;

    /**
     * Indicates that the specified objects (events or Deferred)
     * should be Resolved if this event has occurred.
     */
    public function thenResolve(DeferredInterface ...$objects): static;

    /**
     * Indicates that the specified objects (events or Deferred)
     * should be Rejected if this event has occurred.
     */
    public function thenReject(DeferredInterface ...$objects): static;
}

interface DeferredInterface extends FutureInterface,
    CancellationInterface,
    CompletionPublisherInterface,
    ThenInterface

{
    /**
     * Resolves the deferred object with a result.
     *
     * @param mixed $value The result value.
     * @return void
     */
    public function resolve(mixed $value): void;

    /**
     * Rejects the deferred object with an error.
     *
     * @param \Throwable $error The reason for rejection.
     * @return void
     */
    public function reject(\Throwable $error): void;

    /**
     * Returns the future associated with the deferred object.
     *
     * @return FutureInterface
     */
    public function getFuture(): FutureInterface;
}


abstract class DeferredAbstract implements DeferredInterface
{
    public function getStatus(): int {}

    public function isReady(): bool {}

    public function isFailed(): bool {}

    public function getResult(): mixed {}

    public function getError(): ?\Throwable {}

    public function cancel(): void {}

    public function onSuccess(callable $onFulfilled): void {}

    public function onError(callable $onRejected): void {}

    public function onFinally(callable $onFinally): void {}

    public function thenIgnore(CancellationInterface ...$objects): static {}

    public function thenResolve(DeferredInterface ...$objects): static {}

    public function thenReject(DeferredInterface ...$objects): static {}

    public function resolve(mixed $value): void {}

    public function reject(\Throwable $error): void {}

    public function getFuture(): FutureInterface {}
}

final class DeferredResume extends DeferredAbstract
{
}