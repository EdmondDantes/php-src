--TEST--
Await function
--FILE--
<?php

$fiber = Async\async(function() {
    Async\wait();
    return "result from fiber\n";
});

Async\run(function(Async\FiberHandle $fiber) {
    echo "async function 1\n";
    echo "from fiber1: ".Async\await($fiber);
}, $fiber);

Async\run(function(Async\FiberHandle $fiber) {
    echo "async function 2\n";
    echo "from fiber2: ".Async\await($fiber);
}, $fiber);

Async\run(function(Async\FiberHandle $fiber) {
    echo "async function 3\n";
    echo "from fiber3: ".Async\await($fiber);
}, $fiber);

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
async function 2
async function 3
from fiber1: result from fiber
from fiber2: result from fiber
from fiber3: result from fiber
end
