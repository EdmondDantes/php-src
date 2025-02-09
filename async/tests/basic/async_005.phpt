--TEST--
Async\async() with multiple parameters
--FILE--
<?php

Async\async(function(string $parameter1, array $parameter2, int $parameter3) {
    echo "async function: $parameter1 $parameter3\n";
    foreach ($parameter2 as $key => $value) {
        echo "$key: $value\n";
    }
}, "1", ["key" => "value"], 2);

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function: 1 2
key: value
end
