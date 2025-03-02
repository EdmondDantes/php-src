--TEST--
Run fiber with exception
--FILE--
<?php

Async\async(function() {
    echo "async function 1\n";

    throw new Exception("some error");

})->defer(function() { echo "deferred 1\n"; });

Async\run(function() {
    echo "async function 2\n";
});

echo "start\n";

try {
    Async\launchScheduler();
} catch (Exception $e) {
    echo "Caught exception: ", $e->getMessage(), "\n";
}

echo "end\n";

?>
--EXPECT--
start
async function 1
deferred 1
Caught exception: some error
end
