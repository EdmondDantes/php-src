--TEST--
Async\defer with Async\wait in the scheduler context
--FILE--
<?php

Async\defer(function() {
    echo "async function defer 1\n";
    Async\wait();
});

Async\run(function() {
    echo "async function 2\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function defer 1
async function 2
end
