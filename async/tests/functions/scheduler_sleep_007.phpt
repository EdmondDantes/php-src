--TEST--
A nested call sleep() in multiple nested fibers.
--FILE--
<?php

Async\async(function() {
    echo "fiber start\n";

    Async\async(function() {
        sleep(1);
        echo "fiber 2\n";
    });

    sleep(1);
    echo "fiber end\n";

    Async\async(function() {
        echo "fiber 3\n";
    });
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
fiber start
fiber end
fiber 2
fiber 3
end
