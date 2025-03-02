--TEST--
Sleep inside fiber
--FILE--
<?php

Async\defer(function() {
    echo "defer 1\n";
});

Async\async(function() {
    echo "fiber start\n";
    sleep(1);
    echo "fiber end\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
defer 1
fiber start
fiber end
end
