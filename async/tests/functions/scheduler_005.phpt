--TEST--
Delay in main fiber
--FILE--
<?php

Async\defer(function() {
    echo "defer 1\n";
});

Async\delay(1000, function() {
    echo "timeout 1\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
defer 1
timeout 1
end
