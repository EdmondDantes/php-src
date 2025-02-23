--TEST--
Context: context should be shared between async functions2
--FILE--
<?php

Async\Context::current(true)->setKey('test-key', 'test-value');

Async\run(function() {
    $value = Async\Context::current()->get('test-key');
    echo "async function 1: {$value}\n";
});

Async\run(function() {
    $value = Async\Context::current()->get('test-key');
    echo "async function 2: {$value}\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1: test-value
async function 2: test-value
end
