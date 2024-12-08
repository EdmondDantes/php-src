# Implementation await

When a waiting object is created and passed to the `await()` method, 
it is automatically associated with the current `Fiber`, 
and the `Fiber` is suspended, transferring control to the `Fiber Scheduler`.

A `Fiber` can have only one waiting object at any given time. 
This limitation has several important implications:

* When a `Fiber` is waiting for something, the `Scheduler` knows exactly what it is waiting for.
* If an exception needs to be thrown into the waiting `Fiber`, 
it will automatically cause the await object to transition to its final state.

The `Scheduler` distinguishes between two types of AwaitObjects:

* System `AwaitObjects`, such as I/O streams, timers, and signals.
* User-mode `AwaitObjects`, which are part of the application program, such as `Promises` or `Channels`.

## User-mode AwaitObjects

To implement `User-mode` waiting objects, the `Async` library defines the `FiberDeferred` class, 
which implements the methods:

```php

use Async\Deferred;
use Async\AwaitableInterface;

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

interface DeferredInterface extends AwaitableInterface, CompletionPublisherInterface
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
     * Returns the promise associated with the deferred object.
     *
     * @return PromiseInterface
     */
    public function getPromise(): AwaitableInterface;
}

final class FiberDeferred implements DeferredInterface
{
}
```

As soon as the `FiberDeferred` class is created, 
it is immediately linked to the current `Fiber`. 
From that moment, the user can resolve or reject it.

The `FiberDeferred` class is a fundamental primitive for implementing `Promise` and `Channel`.

### FiberDeferred resolution handling

When a `FiberDeferred` is resolved, it calls the internal method `Scheduler::resumeFiber()`, 
which instructs the scheduler to queue the Fiber for execution.

The Scheduler immediately invokes all CompletionPublisherInterface 
handlers that cannot switch fibers or perform asynchronous operations. 
If this happens, a fatal exception is thrown.

Then the `Scheduler` return control to the current `Fiber` without a context switch.

```plantuml
@startuml
skinparam linetype ortho

|Fiber1|
start
:Fiber create FiberDeferred object;
:Scheduler links FiberDeferred to the current Fiber1;
:Fiber calls await(FiberDeferred);
:Scheduler switches control to its Fiber;


|#AntiqueWhite|Scheduler|
:Execution of microtasks;
:Waiting for new events;
:Switching execution to active Fibers;

|#LightGreen|Fiber2|
:Some work...; 
:Calls **FiberDeferred.resolve()/reject()**;
:Scheduler blocks Fiber switching;
:Scheduler marks the associated Fiber1 as **resumed**;
:Scheduler executes all CompletionPublisherInterface handlers;
:Scheduler returns control to the current Fiber;
:Fiber2 call await();

|Scheduler|
:Execution of microtasks;
:Waiting for new events;
:Switching execution to active Fibers;

|Fiber1|
:Scheduler returns control to Fiber1;

stop
@enduml```

