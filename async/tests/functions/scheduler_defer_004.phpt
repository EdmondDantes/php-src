--TEST--
Defer
--FILE--
<?php

Async\defer(function() {
    echo "defer 1\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
defer 1
end
