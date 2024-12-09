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

## Low-level AwaitObjects

The `Async` component is based on low-level awaitable objects that cannot be implemented at the user-mode level. 
The key class of the library for implementing `Fiber` switching is the `DeferredResume` class.

```php

use Async\Deferred;
use Async\FutureInterface;

interface AwaitableInterface
{
    /**
     * Returns events associated with the awaitable object.
     *
     * @return EventHandlerInterface[]
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

interface IgnorableInterface
{
    public function ignore(): void;
}

interface ThenInterface
{
    public function thenIgnore(IgnorableInterface $object): static;
    
    public function thenResolve(DeferredInterface $object): static;
    
    public function thenReject(DeferredInterface $object): static;
}

interface DeferredInterface extends FutureInterface, ThenInterface, CompletionPublisherInterface
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
     * Ignores the deferred object.
     *
     * @return void
     */
    public function ignore(): void;
    
    /**
     * Returns the future associated with the deferred object.
     *
     * @return FutureInterface
     */
    public function getFuture(): FutureInterface;
}

final class DeferredResume implements DeferredInterface
{
}
```

The `DeferredResume` class describes an awaitable object 
whose resolution indirectly leads to calling the `Fiber::resume()` method.

The `DeferredResume` class requires adherence to the following usage rules:

* A `DeferredResume` object must be created only within the `Fiber` it is intended to resume.
* The `resolve()` and `reject()` methods can be called from user-mode in any `Fiber`.

The following low-level interfaces/classes manage what happens during event processing:

* `EventDescriptorInterface`: Implements an event descriptor and handler that is invoked when the event occurs.
* `DeferredResolverInterface`: Implements the logic for resolving a `Deferred`. 
This interface is typically implemented by `Event objects` that transition the state 
of the `Event` to the state of a Deferred. 

The following classes implement both of these interfaces:

* `InputOutputEvent`  - an event for handling input/output
* `TimerEvent`        - an event for handling timers
* `SignalEvent`       - an event for handling signals
* `FileSystemEvent`   - an event from the file system
* `ProcessEvent`      - an event related to processes
* `ThreadEvent`       - an event related to threads 

Using these primitives, you can create any awaitable object with unique logic.
Let's consider this with an example: `AwaitTimeout`:

```php

use Async\DeferredResume;

final readonly class AwaitTimeout implements AwaitableInterface
{
    private TimerEvent $event;

    public function __construct(int $timeout)
    {
        $this->event = new TimerEvent($timeout, new DeferredResume());
    }
    
    public function getWaitingEvents(): array
    {
        return [$this->event];
    }
}

```

In this example, a `Timer` event descriptor is created, 
which resolves a `DeferredResume` when the timer triggers. 
This is the simplest example of using primitives.

Now, let's consider a more complex example: waiting for read/write operations.

```php

use Async\DeferredResume;

final readonly class AwaitSocket implements AwaitableInterface
{
    private InputOutputEvent $event;
    private TimerEvent $timeout;

    public function __construct(Socket $socket, int $events, int $timeout = 0)
    {
        $this->event    = new InputOutputEvent($stream, $events, new DeferredResume());
        $this->timeout  = null;
        
        if ($timeout > 0) {
            $deferred       = new DeferredResume();
            $this->timeout  = new TimerEvent($timeout, $deferred);
            $this->timeout->thenIgnore($this->event);       
        }        
    }
    
    public function getWaitingEvents(): array
    {
        return [$this->event, $this->timeout];
    }
}

```

## User-mode AwaitObjects

To implement `User-mode` waiting objects, the `Async` library defines the `DeferredResume` class, 
which implements the methods:



As soon as the `DeferredResume` class is created, 
it is immediately linked to the current `Fiber`. 
From that moment, the user can resolve or reject it.

The `DeferredResume` class is a fundamental primitive for implementing `Promise` and `Channel`.

### DeferredResume resolution handling

When a `DeferredResume` is resolved, it calls the internal method `Scheduler::resumeFiber()`, 
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
:Fiber create DeferredResume object;
:Scheduler links DeferredResume to the current Fiber1;
:Fiber calls await(DeferredResume);
:Scheduler switches control to its Fiber;


|#AntiqueWhite|Scheduler|
:Execution of microtasks;
:Waiting for new events;
:Switching execution to active Fibers;

|#LightGreen|Fiber2|
:Some work...; 
:Calls **DeferredResume.resolve()/reject()**;
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

