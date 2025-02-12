--TEST--
shell_exec() inside async function
--SKIPIF--
<?php
if (!function_exists("shell_exec")) echo "skip shell_exec() is not available";
?>
--FILE--
<?php

Async\run(function () {
    $php = getenv("TEST_PHP_EXECUTABLE");
    $cat = shell_exec("$php -r \"sleep(1); echo 'hello';\"");
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