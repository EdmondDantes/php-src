--TEST--
Async\defer exception: Cannot await in the scheduler context
--FILE--
<?php

Async\defer(function() {
    echo "async function defer 1\n";
    Async\wait();
});

echo "start\n";
try {
    Async\launchScheduler();
} catch (Exception $e) {
    echo "Exception: ", $e->getMessage(), "\n";
}

echo "end\n";

?>
--EXPECT--
start
async function defer 1
Exception: Cannot await in the scheduler context
end
