--TEST--
Testing the await() function
--FILE--
<?php

Async\wait();

echo "Success\n";
?>
--EXPECTF--
Success
