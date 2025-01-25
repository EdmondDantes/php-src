--TEST--
Async\await()
--FILE--
<?php

Async\async(function() {
    echo "async function 1:1\n";
    Async\await();
    echo "async function 1:2\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1:1
async function 1:2
end
