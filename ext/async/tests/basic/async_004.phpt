--TEST--
CancellationException should be ignored
--FILE--
<?php

Async\async(function() {
    echo "async function 1\n";

    throw new Async\CancellationException("Should be ignored");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
end
