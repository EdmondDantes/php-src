--TEST--
Delay: Async\AsyncException: Reactor is not running
--FILE--
<?php

Async\defer(function() {
    echo "defer 1\n";
});

try {
    Async\delay(1000, function() {
        echo "timeout 1\n";
    });
} catch (Async\AsyncException $e) {
    echo "Async\AsyncException: ", $e->getMessage(), "\n";
}

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
Async\AsyncException: Reactor is not running
start
defer 1
end
