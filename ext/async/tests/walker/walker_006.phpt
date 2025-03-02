--TEST--
Async\Walker::walk with generator
--FILE--
<?php

function generator() {
    for ($i = 1; $i <= 3; $i++) {
        yield $i;
    }
}

Async\Walker::walk(generator(), function(mixed $value) {
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
value: 1
value: 2
value: 3
async function 2
end
