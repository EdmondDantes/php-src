--TEST--
Nested async functions order
--FILE--
<?php

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    echo "value: $value\n";
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
echo: value: 1
echo: value: 2
echo: value: 3
async function 2
end
