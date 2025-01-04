--TEST--
Cannot await a resume object outside of an async context
--FILE--
<?php

try {
    Async\await(new Async\Resume());
} catch (Async\AsyncException $e) {
    echo "Async\AsyncException should be catch";
}
?>
--EXPECT--
Async\AsyncException should be catch
