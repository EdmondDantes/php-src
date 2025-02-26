--TEST--
Scheduler with async function
--FILE--
<?php

Async\async(function() {
    echo "async function 1\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
end
