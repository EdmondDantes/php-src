--TEST--
Future: awaitFirst() - basic usage
--FILE--
<?php

$futureState1 = new Async\FutureState();
$futureState2 = new Async\FutureState();

$futures = [new Async\Future($futureState1), new Async\Future($futureState2)];

Async\run(function() use($futures) {
    $result = Async\awaitFirst($futures);

    echo "awaitFirst: result: $result\n";
});

Async\run(function() use($futureState1) {
    $futureState1->complete("future 1");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
awaitFirst: result: future 1
end
