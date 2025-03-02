--TEST--
Two Async\Walker::walk with array and many fibers
--FILE--
<?php

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    echo "array 1: $value\n";
    Async\wait();
});

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    echo "array 2: $value\n";
    Async\wait();
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
array 1: 1
array 2: 1
array 1: 2
array 1: 3
array 2: 2
array 2: 3
end
