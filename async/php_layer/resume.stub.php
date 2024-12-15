<?php

/** @generate-class-entries */

namespace Async;

/**
 * A class that links an array of events to the resumption of a `Fiber`.
 *
 * It associates the moment an event occurs with the logic by which the `Fiber` will be resumed.
 * This is a key element for transferring control between `Fibers`.
 *
 * A `ResumeDescriptor` object cannot be created outside the current `Fiber`.
 * The `ResumeDescriptor` object can transition to a resolved state only once,
 * after which it must be destroyed.
 */
final class ResumeDescriptor
{
    /**
     * Resumes the fiber with a value.
     */
    public function resume(mixed $value = null): void {};

    /**
     * Throws an exception into the fiber.
     */
    public function throw(?\Throwable $error = null): void {};

    public function isResolved(): bool {};

    public function getEventDescriptors(): array {}

    public function when(EventDescriptorInterface $event, callable $callback): static {}

    public function resumeWhen(EventDescriptorInterface $event): static {}

    public function throwWhen(EventDescriptorInterface $event): static {}
}
