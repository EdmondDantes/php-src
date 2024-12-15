<?php

/** @generate-class-entries */

namespace Async;

interface CallbackInterface
{
    public function disposeCallback(): void;
}

interface EventHandleInterface
{
    public function addCallback(CallbackInterface $callback): static;
}
