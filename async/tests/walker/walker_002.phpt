--TEST--
Async\Walker::walk with array and many fibers
--FILE--
<?php

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    echo "value: $value\n";
    Async\wait();
});

Async\run(function() {
    echo "async function 2\n";

    Async\run(function() {
        echo "async function 3\n";
    });
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
value: 1
async function 2
value: 2
value: 3
async function 3
end
