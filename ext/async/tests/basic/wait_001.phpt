--TEST--
Testing the wait() function
--FILE--
<?php

Async\wait();

echo "Success\n";
?>
--EXPECTF--
Success
