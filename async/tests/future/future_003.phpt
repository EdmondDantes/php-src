--TEST--
Future: chaining maps with no return value
--FILE--
<?php

Async\async(function() {

    $futureState = new Async\FutureState();
    $future = new Async\Future($futureState);

    $future->map(function(string $value) {
        echo "future map1: $value\n";
        return "map1: return";
    })->map(function(string $value) {
        echo "future map2 after 1: $value\n";
    })->map(function(mixed $value) {
        if($value === null) {
            $value = "value: NULL";
        }

        echo "future map3 after 2: $value\n";
    });

    $future->map(function(string $value) {
        echo "future last: $value\n";
    });

    $futureState->complete("complete");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
future map1: complete
future map2 after 1: map1: return
future map3 after 2: value: NULL
future last: complete
end