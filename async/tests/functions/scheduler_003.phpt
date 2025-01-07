--TEST--
Scheduler with async functions
--FILE--
<?php

Async\async(function() {
    echo "async function 1\n";
});

Async\async(function() {
    echo "async function 2\n";
});

Async\async(function() {
    echo "async function 3\n";
});


echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
async function 2
async function 3
end
