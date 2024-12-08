# Task Scheduler

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

function await(AwaitableInterface $awaitable = null): void;

```

The `await` method takes a single optional parameter — an awaitable object.
The `AwaitableInterface` is defined as follows:

```php
interface AwaitableInterface
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
* `AwaitSignal`         — waits for an operating system signal.
* `AwaitEvent`          — waits for an event.
* `AwaitChannel`        — waits for a data transfer channel.
* `AwaitComposite`      — waits for a set of awaitable objects.