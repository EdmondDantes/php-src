<?php

/** @generate-class-entries */

namespace Async;

enum FutureStatus: int
{
    case PENDING    = 0;
    case RESOLVED   = 1;
    case REJECTED   = 2;
    case CANCELLED  = 3;
}

/**
 * Describes an object that provides a deferred result.
 */
interface FutureInterface
{
    public function getStatus(): FutureStatus;

    public function isReady(): bool;

    public function isFailed(): bool;

    public function getResult(): mixed;

    public function getError(): ?\Throwable;
}

/**
 * The `CancellationInterface` interface is used to ignore the result of an awaitable object.
 */
interface CancellationInterface
{
    public function cancel(): void;
}

/**
 * The `EventDescriptorInterface` interface is used to implement an event descriptor
 * and handler that is invoked when the event occurs.
 */
interface EventDescriptorInterface extends CancellationInterface
{
    /**
     * Called when the event was registered in the event loop.
     */
    public function onRegistered(callable $disposeCallback): void;
}

/**
 * The `AwaitableInterface` interface is used to implement an awaitable object.
 */
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

/**
 * The `CompletionPublisherInterface` interface is used to implement a completion publisher
 * that notifies subscribers when the operation is completed.
 */
interface CompletionPublisherInterface
{
    /**
     * Registers a subscriber for success events.
     *
     * @param callable $onFulfilled The success callback.
     * @return void
     */
    public function onSuccess(callable $onFulfilled): static;

    /**
     * Registers a subscriber for error events.
     *
     * @param callable $onRejected The error callback.
     * @return void
     */
    public function onError(callable $onRejected): static;

    /**
     * Registers a subscriber for finalization events.
     *
     * @param callable $onFinally The finalization callback.
     * @return void
     */
    public function onFinally(callable $onFinally): static;
}

/**
 * The `ThenInterface` interface is used to register callbacks for completion events.
 */
interface ThenInterface
{
    /**
     * Indicates that the specified objects (events or Deferred)
     * should be ignored if this event has occurred.
     */
    public function thenCancel(CancellationInterface ...$objects): static;

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

/**
 * An interface for objects whose state can be changed once from undefined to resolve.
 */
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



abstract class CompletionPublisherAbstract implements CompletionPublisherInterface
{
    public function onSuccess(callable $onFulfilled): void {}

    public function onError(callable $onRejected): void {}

    public function onFinally(callable $onFinally): void {}

    public function thenCancel(CancellationInterface ...$objects): static {}

    public function thenResolve(DeferredInterface ...$objects): static {}

    public function thenReject(DeferredInterface ...$objects): static {}

    final protected function invokeCompletionHandlers(FutureStatus $status): void {}
}

/**
 * The Base class for implementing the DeferredInterface.
 * It can be used to create a custom Deferred object.
 */
abstract class DeferredAbstract extends CompletionPublisherAbstract implements DeferredInterface
{
    public function getStatus(): FutureStatus {}

    public function isReady(): bool {}

    public function isFailed(): bool {}

    public function getResult(): mixed {}

    public function getError(): ?\Throwable {}

    public function cancel(): void {}

    public function resolve(mixed $value): void {}

    public function reject(\Throwable $error): void {}

    public function getFuture(): FutureInterface {}

    /**
     * Internal handler for the Deferred object.
     * This method is called when the Deferred object is resolved or rejected.
     */
    abstract protected function internalHandler(): void;
}

/**
 * The key class of the library for implementing `Fiber` switching is the `DeferredResume` class.
 */
final class DeferredResume extends DeferredAbstract
{
    public function __construct() {}

    protected function internalHandler(): void {}
}

/**
 *
 */
class InputOutputEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

final class TimerEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

final class SignalEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

final class FileSystemEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

final class ProcessEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

final class ThreadEvent extends CompletionPublisherAbstract implements EventDescriptorInterface
{
    public function cancel(): void {}

    public function onRegistered(callable $disposeCallback): void {}
}

function await(AwaitableInterface $awaitable): void {};

function async(callable $callback): FutureInterface {};

function defer(callable $task): void {};

function delay(int $timeout, callable $task): void {};

function repeat(int $interval, callable $task): void {};

function onSignal(int $sigNumber, callable $task): void {};
