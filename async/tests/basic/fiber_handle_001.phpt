--TEST--
Fiber handle with callback
--FILE--
<?php

$callback = new Async\Closure(function(Async\Notifier $notifier, \Fiber $fiber) {
    echo "Fiber was terminated {$notifier->toString()}\n";
});

$fiber = Async\async(function() {
    echo "async function 1\n";
});

$fiber->addCallback($callback);

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
Fiber was terminated Async\FiberHandle
end
