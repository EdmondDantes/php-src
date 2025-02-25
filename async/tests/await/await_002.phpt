--TEST--
Future: awaitAnyN - basic usage
--FILE--
<?php

$futureState1 = new Async\FutureState();
$futureState2 = new Async\FutureState();

$futures = [new Async\Future($futureState1), new Async\Future($futureState2)];

Async\run(function() use($futures) {
    [$results, ] = Async\awaitAnyN(1, $futures);

    echo "awaitAnyN: result: {$results[0]}\n";
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
awaitAnyN: result: future 1
end
