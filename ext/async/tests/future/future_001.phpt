--TEST--
Future: basic test
--FILE--
<?php

Async\async(function() {

    $futureState = new Async\FutureState();
    $future = new Async\Future($futureState);

    $future->map(function(string $value) {
        echo "future map: $value\n";
    });

    $futureState->complete("complete");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
future map: complete
end
