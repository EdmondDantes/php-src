--TEST--
Async\defer
--FILE--
<?php

Async\defer(function() {
    echo "async function defer\n";
});

Async\async(function() {
    echo "async function 1\n";
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
async function defer
async function 1
async function 2
end
