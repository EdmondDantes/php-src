--TEST--
Fiber handle cancelWith() method
--FILE--
<?php

$fiber = Async\async(function() {
    echo "async function 1:1\n";
    Async\await();
    echo "async function 1:2\n";
});

Async\run(function() use($fiber) {
    echo "async function 2\n";
    $fiber->cancelWith(new Async\CancellationException("Should be ignored"));
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1:1
async function 2
end
