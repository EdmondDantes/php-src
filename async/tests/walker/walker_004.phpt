--TEST--
Async\Walker::walk with delay
--FILE--
<?php

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    usleep(10 - $value * 2);
    echo "value: $value\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
value: 3
value: 2
value: 1
end
