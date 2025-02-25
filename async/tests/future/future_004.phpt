--TEST--
Future: chaining maps with error
--FILE--
<?php

Async\async(function() {

    $futureState = new Async\FutureState();
    $future = new Async\Future($futureState);

    $future->map(function(string $value) {
        echo "future map1: $value\n";
    })->catch(function(Exception $error) {
        echo "catch error from 2: {$error->getMessage()}\n";
    });

    $future->catch(function(Exception $error) {
        echo "catch error from 1: {$error->getMessage()}\n";
    });

    $futureState->error(new Exception("error"));
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
catch error from 2: error
catch error from 1: error
end