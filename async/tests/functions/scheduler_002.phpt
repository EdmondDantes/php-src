--TEST--
launchScheduler empty start
--FILE--
<?php

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
end
