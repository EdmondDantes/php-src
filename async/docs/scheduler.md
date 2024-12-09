# Task Scheduler

## Basic interfaces

### FutureInterface

The `FutureInterface` interface is the basis for all awaitable objects.

```php
interface FutureInterface
{
    public function getStatus(): int;
    public function isReady(): bool;
    public function isFailed(): bool;
    public function getResult(): mixed;
    public function getError(): ?Throwable;
}
```

### IgnorableInterface

The `IgnorableInterface` interface is used to ignore the result of an awaitable object.

```php

interface IgnorableInterface
{
    public function ignore(): void;
}

```

### EventDescriptorInterface

The `EventDescriptorInterface` interface is used to implement an event descriptor
and handler that is invoked when the event occurs.

```php

interface EventDescriptorInterface extends IgnorableInterface
{

}
```

### AwaitableInterface

```php
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
     * @return EventHandlerInterface[]
     */
    public function getWaitingEvents(): array;    
}
```

### CompletionPublisherInterface

The `CompletionPublisherInterface` interface is used to register callbacks for completion events.

```php
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
```

### ThenInterface

The `ThenInterface` interface is used to register callbacks for completion events.

```php
interface ThenInterface
{
    /**
     * Indicates that the specified objects (events or Deferred) 
     * should be ignored if this event has occurred.
     */
    public function thenIgnore(IgnorableInterface ...$objects): static;
    
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
```

### DeferredInterface

```php
interface DeferredInterface extends FutureInterface, 
                                    IgnorableInterface, 
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
     * @param Throwable $error The reason for rejection.
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
```

## Transfer of Control Between Fibers

In `PHP`, the mechanism for switching between fibers is still entirely left to the user-level programmer. 
`PHP` itself does not provide any high-level abstractions for managing concurrent execution.

The `Async` component changes this by introducing additional ways to manage concurrency.

Async defines a `Scheduler` component that implements an event loop and manages 
the process of switching between `Fibers`.

The `Scheduler` is responsible for implementing methods to transfer control between `Fibers`, 
as well as tracking events and activating `Fibers`.

To achieve this, the `Scheduler` implements the `await()` method, 
which blocks the execution of the current Fiber until certain conditions are met.

The general prototype of the `await()` function:
```php

function await(FutureInterface $awaitable = null): void;

```

The `await` method takes a single optional parameter — an awaitable object.
The `FutureInterface` is defined as follows:

```php
interface FutureInterface
{
    public function getStatus(): int;
    public function isReady(): bool;
    public function isFailed(): bool;
    public function getResult(): mixed;
    public function getError(): ?Throwable;
}
```

If the `await` function is called without parameters, the `Fiber` voluntarily yields control to 
the `Scheduler` but will be resumed as soon as an opportunity arises.
In other words, an empty await works like a tick() function.

## Scheduler

The Scheduler is a special function that runs in its own Fiber. 
It always gains control when other Fibers yield it by calling await.

## Awaitable Objects

The `Async` provides several different awaitable objects that make development more convenient:

* `AwaitInputOutput`    — waits for input/output with timeout and cancellation.
* `AwaitTimer`          — waits for a timer.
* `AwaitFuture`         — waits for a future.
* `AwaitCancellation`   — waits for a cancellation signal.
* `AwaitSignal`         — waits for an operating system signal.
* `AwaitChannel`        — waits for a data transfer channel.
* `AwaitComposite`      — waits for a set of awaitable objects.

## Microtasks

`Microtasks` are a special type of primitives in the context of the `Scheduler`, 
an idea originating from the implementation of `JavaScript`. 
`Microtasks` are specialized **high-priority** code that is **GUARANTEED** to execute before 
any `I/O event` handling code. 
`Microtasks` should take the minimal possible time and ideally should not create 
any asynchronous operations.

To save resources on `Fiber` context switching, the `Scheduler` uses 
a shared `Fiber` context for executing all microtasks. 
Therefore, if `microtasks` execute quickly and do not call `await()` within themselves, 
no Fiber switching occurs.

`Microtasks` are executed directly within the `Scheduler's Fiber`, meaning they can disrupt the event loop. 
If a `microtask` calls `await` within itself, this triggers specific system behavior.

A `microtask` calls await and becomes a `macrotask`:

1. The `Scheduler` detects an attempt to call await within the context of the `Scheduler's Fiber`.
2. The `Scheduler` creates a new `Fiber` for the `Scheduler`, while the current `Fiber` loses its `Scheduler` status 
and becomes a `Fiber` for the `microtask`, essentially turning into a `macrotask`.
3. Control is then returned to the Scheduler's Fiber.

The `Scheduler` provides a method to create `microtasks` explicitly:

```php
function defer(callable|DeferredTaskInterface $task): void;
function delay(int $timeout, callable|DeferredTaskInterface $task): void;
function repeat(RepeatableCallInterface $task): void;
function onSignal(int $signo, callable|DeferredTaskInterface $task): void;
```

These three methods allow explicit creation of microtasks that will be 
executed immediately when control is handed over to the `Scheduler`. 

All of them will not create new Fiber objects but will be executed within the `Scheduler`.

A microtask of the defer type has the highest execution priority and is **GUARANTEED** 
to execute before the `Scheduler` transitions to event waiting.

Microtasks involving timers, signals, and other types of events will execute **BEFORE** I/O tasks 
but after polling the event queue.

## Promise and Deferred primitives

The `Promise`/`Deferred` classes are fundamental primitives implementing the `FutureInterface`. 
The `Deferred` class represents an object that can transition into a completed state with either a result or an error, 
while the `Promise` represents its readonly state.

## Cancellation

The `Cancellation` primitive is a special type of awaitable object that allows you to cancel the execution or waiting.
In essence, a `Cancellation` is a type of Promise that can be resolved to a completed state only once.

## Channels

`Channels` are a special type of awaitable object that allows you to transfer data between Fibers.

`Channels` are objects that logically consist of two `EndPoints` – points for receiving and sending messages. 
`Fibers` act as the owners of an EndPoint. 
When a `channel` is created, the current Fiber automatically becomes the owner of 
the **writer `EndPoint`** and gains the ability to send data.

As soon as another `Fiber` attempts to read data, it automatically becomes the owner of the **reader EndPoint**.

To prevent complex errors, `Async` does not allow the read methods to be called by 
a Fiber owning the writer EndPoint, nor does it allow 
a Fiber owning the read EndPoint to attempt writing to the channel.

You can retrieve information about which `Fibers` are bound to the read or write `EndPoints` of a `channel`.

If you need two-way communication between `Fibers`, create two separate `channels`.

## Timers

