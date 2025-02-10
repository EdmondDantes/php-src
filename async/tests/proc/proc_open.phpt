--TEST--
proc_open
--SKIPIF--
<?php
if (!function_exists("proc_open")) echo "skip proc_open() is not available";
?>
--FILE--
<?php

Async\run(function () {

    echo "fiber1 start\n";

    $ds = array(
            0 => array("pipe", "r"),
            1 => array("pipe", "w"),
            2 => array("pipe", "w")
            );

    $php = getenv("TEST_PHP_EXECUTABLE");
    $cat = proc_open(
            [$php, "-r", "usleep(10000); echo 'hello';"],
            $ds,
            $pipes
            );

    proc_close($cat);

    echo "fiber1 end\n";
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
fiber1 start
async function 2
fiber1 end
end