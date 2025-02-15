--TEST--
Await function
--FILE--
<?php

$fiber = Async\async(function() {
    Async\wait();
    throw new Exception("error from fiber");
});

Async\run(function(Async\FiberHandle $fiber) {
    echo "async function 1\n";

    try {
        Async\await($fiber);
    } catch (\Throwable $e) {
        echo "from fiber1: ".$e->getMessage()."\n";
    }

}, $fiber);

Async\run(function(Async\FiberHandle $fiber) {
    echo "async function 2\n";

    try {
        Async\await($fiber);
    } catch (\Throwable $e) {
        echo "from fiber2: ".$e->getMessage()."\n";
    }

}, $fiber);

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
async function 2
from fiber1: error from fiber
from fiber2: error from fiber
end
