--TEST--
Future: chaining maps
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
        return "map1: return";
    })->map(function(string $value) {
        echo "future map3 after 2: $value\n";
    })->map(function(string $value) {
        echo "future map4 after 3: $value\n";
    });

    $future->map(function(string $value) {
        echo "future map2: $value\n";
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
future map2 after 1: complete
future map3 after 2: complete
future map4 after 3: complete
future map2: complete
end