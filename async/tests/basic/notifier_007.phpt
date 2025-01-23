--TEST--
Fiber handle with callback
--FILE--
<?php

$callback = new Async\Callback(function(Async\Notifier $notifier, mixed $data) {
    echo "callback from $notifier: $data\n";
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
end
