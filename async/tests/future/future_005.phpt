--TEST--
Future: call map and catch with error
--FILE--
<?php

Async\async(function() {

    $futureState = new Async\FutureState();
    $future = new Async\Future($futureState);

    $future->map(function(string $value) {
        echo "future: $value\n";
        throw new Exception("error");
    })->catch(function(Exception $error) {
        echo "catch error: {$error->getMessage()}\n";
    });

    $futureState->complete("complete");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
future: complete
catch error: error
end