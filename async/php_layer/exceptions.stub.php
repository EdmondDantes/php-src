<?php

/** @generate-class-entries */

namespace Async;

/**
 * Exception thrown when a Fiber is canceled.
 * Code inside the Fiber must properly handle this exception to ensure graceful termination.
 */
class CancellationException extends \Exception {}

/**
 * Common type of exception.
 */
class AsyncException extends \Exception {}

/**
 * General exception for input/output operations.
 * Can be used with sockets, files, pipes, and other I/O descriptors.
 */
class InputOutputException extends \Exception {}

/**
 * Exception thrown when a timeout occurs.
 */
class TimeoutException extends \Exception {}

/**
 * Exception thrown when a poll operation fails.
 */
class PollException extends \Exception {}