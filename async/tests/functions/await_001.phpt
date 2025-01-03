--TEST--
Testing the await() function
--FILE--
<?php

Async\await();

echo "Success\n";
?>
--EXPECTF--
Success
